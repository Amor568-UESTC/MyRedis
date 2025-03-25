#pragma once

#include<cassert>
#include<cstring>
#include<vector>
#include<string>
#include<sys/uio.h>
#include<atomic>

struct BufferSequence
{
    static const size_t kMaxIovec=16;
    iovec buffers[kMaxIovec];
    size_t cnt;

    size_t TotalBytes() const
    {
        assert(cnt<=kMaxIovec);
        size_t nBytes=0;
        for(std::size_t i=0;i<cnt;i++)
            nBytes+=buffers[i].iov_len;
        return nBytes;
    }
};

inline size_t RoundUp2Power(size_t size)
{
    if(size==0) return 0;
    size_t roundSize=1;
    while(roundSize<size)
        roundSize<<=1;
    return roundSize;
}

template<typename BUFFER>
class CircularBuffer
{
private:
    std::atomic<size_t> readPos_;
    std::atomic<size_t> writePos_;
    BUFFER buffer_;
    bool owned_=0;

protected:
    size_t maxSize_;

public:
    explicit CircularBuffer(size_t size=0):
    maxSize_(size),readPos_(0),writePos_(0) {}

    CircularBuffer(const BufferSequence& bf);
    CircularBuffer(char*, std::size_t);
    ~CircularBuffer();

    bool IsEmpty() const { return writePos_==readPos_;}
    bool IsFull() const { return ((writePos_+1)&(maxSize_-1))==readPos_;}

    void GetDatum(BufferSequence& buffer,size_t maxSize,size_t offset=0);
    void GetSpace(BufferSequence& buffer,size_t offset=0);

    bool PushData(const void* pData,size_t nSize);
    bool PushDataAt(const void* pData,size_t nSize,size_t offset=0);

    bool PeekData(void* pBuf,size_t nSize);
    bool PeekDataAt(void* pBuf,size_t nSize,size_t offset=0);

    char* ReadAddr() { return &buffer_[readPos_];}
    char* WriteAddr() { return &buffer_[writePos_];}

    void AdjustWritePtr(size_t size);
    void AdjustReadPtr(size_t size);

    size_t ReadbleSize() const { return (writePos_-readPos_);}
    size_t WritableSize() const { return maxSize_-ReadbleSize();}

    std::size_t Capacity() const { return maxSize_;}
    void InitCapacity(size_t size);

    template<typename T>
    CircularBuffer& operator<<(const T& data);
    template<typename T>
    CircularBuffer& operator>>(T& data);

    template <typename T>
    CircularBuffer& operator<<(const std::vector<T>&);
    template <typename T>
    CircularBuffer& operator>>(std::vector<T>&);

    CircularBuffer& operator<<(const std::string& str);
    CircularBuffer& operator>>(std::string& str);
};

template <typename BUFFER>
void CircularBuffer<BUFFER>::GetDatum(BufferSequence& buffer,size_t maxSize,size_t offset)
{
    if(maxSize==0||offset>=ReadbleSize())
    {
        buffer.cnt=0;
        return ;
    }
    assert(readPos_<maxSize_);
    assert(writePos_<maxSize_);

    size_t idx=0;
    const size_t readPos=(readPos_+offset)&(maxSize_-1);
    const size_t writePos=writePos_;
    assert(readPos!=writePos);

    buffer.buffers[idx].iov_base=&buffer_[readPos];
    if(readPos<writePos)
        buffer.buffers[idx].iov_len=min(maxSize,writePos-readPos);
    else
    {
        size_t nLeft=maxSize;
        if(nLeft>maxSize_-readPos)
            nLeft=maxSize_-readPos;
        buffer.buffers[idx].iov_len=nLeft;
        nLeft=maxSize-nLeft;

        if(nLeft>0&&writePos>0)
        {
            if(nLeft>writePos) 
                nLeft=writePos;
            idx++;
            buffer.buffers[idx].iov_base=&buffer_[0];
            buffer.buffers[idx].iov_len=nLeft;
        }
    }
    buffer.cnt=idx+1;
}

template<typename BUFFER>
void CircularBuffer<BUFFER>::GetSpace(BufferSequence& buffer,size_t offset)
{
    assert(readPos_>=0&&readPos_<maxSize_);
    assert(writePos_>=0&&writePos_<maxSize_);
    if(WritableSize()<=offset+1)
    {
        buffer.cnt=0;
        return ;
    }

    size_t idx=0;
    const size_t readPos=readPos_;
    const size_t writePos=(writePos_+offset)&(maxSize_-1);
    buffer.buffers[idx].iov_base=&buffer_[writePos];

    if(readPos>writePos)
    {
        buffer.buffers[idx].iov_len=readPos-writePos-1;
        assert(buffer.buffers[idx].iov_len>0);
    }
    else 
    {
        buffer.buffers[idx].iov_len=maxSize_-writePos;
        if(readPos==0)
            buffer.buffers[idx].iov_len-=1;
        else if(readPos>1)
        {
            idx++;
            buffer.buffers[idx].iov_base=&buffer_[0];
            buffer.buffers[idx].iov_len=readPos-1;
        }
    }
    buffer.cnt=idx+1;
}


template<typename BUFFER>
bool CircularBuffer<BUFFER>::PushData(const void* pData,size_t nSize)
{
    if(!PushDataAt(pData,nSize))
        return 0;
    AdjustWritePtr(nSize);
    return 1;
}

template<typename BUFFER>
bool CircularBuffer<BUFFER>::PushDataAt(const void* pData,size_t nSize,size_t offset)
{
    if(!pData||nSize==0) return 1;
    if(offset+nSize>WritableSize()) return 0;

    const size_t readPos=readPos_;
    const size_t writePos=(writePos_+offset)&(maxSize_-1);
    if(readPos>writePos)
    {
        assert(readPos-writePos>nSize)
        memcpy(&buffer_[writePos],pData,nSize);
    }
    else 
    {
        size_t availBytes1=maxSize_-writePos;
        size_t availBytes2=readPos;
        assert(availBytes1+availBytes2>=1+nSize);

        if(availBytes1>=nSize+1)
            memcpy(&buffer_[writePos],pData,nSize);
        else 
        {
            memcpy(&buffer_[writePos],pData,availBytes1);
            int bytesLeft=static_cast<int>(nSize-availBytes1);
            if(bytesLeft>0)
            memcpy(&buffer_[0],static_cast<const cahr*>(pData)+availBytes1,bytesLeft);
        }
    }
    return 1;
}

template<typename BUFFER>
bool CircularBuffer<BUFFER>::PeekData(void* pBuf,size_t nSize)
{
    if(PeekData(pBuf,nSize))
        AdjustReadPtr(nSize);
    else 
        return 0;
    return 1;
}

template<typename BUFFER>
bool CircularBuffer<BUFFER>::PeekDataAt(void* pBuf,size_t nSize,size_t offset)
{
    if(!pBuf||nSize==0) return 1;
    if(nSize+offset>ReadbleSize()) return 0;

    const size_t writePos=writePos_;
    const size_t readPos=(readPos_+offset)&(maxSize_-1);
    if(readPos<writePos)
    {
        assert(writePos-readPos>=nSize);
        memcpy(pBuf,&buffer_[readPos],nSize);
    }
    else 
    {
        assert(readPos>writePos);
        size_t availBytes1=maxSize_-readPos;
        size_t availBytes2=writePos;
        assert(availBytes1+availBytes2>=nSize);
        
        if(availBytes1>=nSize)
            memcpy(pBuf,&buffer_[readPos],nSize);
        else 
        {
            memcpy(pBuf,&buffer_[readPos],availBytes1);
            assert(nSize-availBytes1>0);
            memcpy(static_cast<char*>(pBuf)+availBytes1,&buffer_[0],nSize-availBytes1);
        }
    }
    return 1;
}

template<typename BUFFER>
inline void CircularBuffer<BUFFER>::AdjustWritePtr(size_t size)
{
    size_t writePos=writePos_;
    writePos+=size;
    writePos&=maxSize_-1;
    writePos_=writePos;
}

template<typename BUFFER>
inline void CircularBuffer<BUFFER>::AdjustReadPtr(size_t size)
{
    size_t readPos=readPos_;
    readPos+=size;
    readPos&=maxSize_-1;
    readPos_=readPos;
}

template<typename BUFFER>
inline void CircularBuffer<BUFFER>::InitCapacity(std::size_t size)
{
    assert(size>0&&size<=1*1024*1024*1024);
    maxSize_=RoundUp2Power(size);
    buffer_.resize(maxSize_);
    std::vector<char>(buffer_).swap(buffer_);
}

template<typename BUFFER>
template<typename T>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator<<(const T& data)
{
    if(!PushData(&data,sizeof(data)))
        assert(!!!"Please modify the DEFAULT_BUFFER_SIZE");
    return *this;
}

template<typename BUFFER>
template<typename T>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator>>(T& data)
{
    if(!PeekData(&data,sizeof(data)))
        assert(!!!"Not enough data in buffer_");
    return *this;
}

template<typename BUFFER>
template<typename T>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator<<(const std::vector<T>& v)
{
    if(!v.empty())
    {
        (*this)<<static_cast<unsigned short>(v.size());
        for(auto& it=v.begin();it!=v.end();it++)
            (*this)<<*it;
    }
    return *this;
}


template<typename BUFFER>
template<typename T>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator>>(std::vector<T>& v)
{
    v.clear();
    unsigned short size;
    *this>>size;
    v.reserve(size);
    while(size--)
    {
        T t;
        *this>>t;
        v.push_back(t);
    }
    return *this;
}

template<typename BUFFER>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator<<(const std::string& str)
{
    *this<<static_cast<unsigned short>(str.size());
    if(!PushData(str.data(),str.size()))
    {
        AdjustWritePtr(static<int>(0-sizeof(unsigned short)));
        assert(!!!"2Please modify the DEFUALT_BUFFER_SIZE");
    }
    return *this;
}

template<typename BUFFER>
inline CircularBuffer<BUFFER>& CircularBuffer<BUFFER>::operator>>(std::string& str)
{
    unsigned short size=0;
    *this>>size;
    str.clear();
    str.reserve(size);
    
    char ch;
    while(size--)
    {
        *this>>ch;
        str+=ch;
    }
    return *this;
}


typedef CircularBuffer<std::vector<char>> Buffer;

template<>
inline Buffer::CircularBuffer(size_t maxSize):
maxSize_(RoundUp2Power(maxSize)),readPos_(0),writePos_(0),buffer_(maxSize_)
{ assert((maxSize_&(maxSize_-1))==0&&"maxSize_ MUST BE power of 2");}

template<int N>
class StackBuffer:public CircularBuffer<char [N]>
{
    using CircularBuffer<char [N]>::maxSize_;

public:
    StackBuffer()
    {
        maxSize_=N;
        if(maxSize_<0) maxSize_=-1;
        if((maxSize_&(maxSize_-1))!=0)
            maxSize_=RoundUp2Power(maxSize_);
        assert((maxSize_&(maxSize_-1))==0&&"maxSize_ MUST BE power of 2");
    }
};


typedef CircularBuffer<char*> AttachedBuffer;

template<>
inline AttachedBuffer::CircularBuffer(char* pBuf,size_t len):
maxSize_(RoundUp2Power(len+1)),readPos_(0),writePos_(0)
{
    buffer_=pBuf;
    owned_=0;
}

template<>
inline AttachedBuffer::CircularBuffer(const BufferSequence& bf):
readPos_(0),writePos_(0)
{
    owned_=0;

    if(bf.cnt==0)
        buffer_=0;
    else if(bf.cnt==1)
    {
        buffer_=(char*)bf.buffers[0].iov_base;
        writePos_=static_cast<int>(bf.buffers[0].iov_len);
    }
    else if(bf.cnt>1)
    {
        owned_=1;
        buffer_=new char[bf.TotalBytes()];

        size_t off=0;
        for(size_t i=0;i<bf.cnt;i++)
        {
            memcpy(buffer_+off,bf.buffers[i].iov_base,bf.buffers[i].iov_len);
            off+=bf.buffers[i].iov_len;
        }
        writePos_=bf.TotalBytes();
    }
    maxSize_=bf.TotalBytes();
}

template<>
inline AttachedBuffer::~CircularBuffer() 
{ 
    if(owned_) delete []buffer_;
}

template<typename T>
inline void OverwriteAt(void* addr,T data) { memcpy(addr,&data,sizeof(data));}
