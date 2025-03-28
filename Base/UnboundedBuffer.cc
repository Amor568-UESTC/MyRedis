#include<iostream>
#include<limits>
#include<cassert>

#include"UnboundedBuffer.h"

namespace myredis
{

const size_t UnboundedBuffer::MAX_BUFFER_SIZE=std::numeric_limits<size_t>::max()/2;

void UnboundedBuffer::AssureSpace_(size_t size)
{
    if(size<=WritableSize()) return ;
    size_t maxSize=buffer_.size();

    while(size>WritableSize()+readPos_)
    {
        if(maxSize<64) maxSize=64;
        else if(maxSize<=UnboundedBuffer::MAX_BUFFER_SIZE)
            maxSize+=maxSize/2;
        else break;
        buffer_.resize(maxSize);
    }

    if(readPos_>0)
    {
        size_t dataSize=ReadableSize();
        std::cout<<dataSize<<" bytes moved from"<<readPos_<<std::endl;
        memmove(&buffer_[0],&buffer_[readPos_],dataSize);
        readPos_=0;
        writePos_=dataSize;
    }
}

size_t UnboundedBuffer::PushDataAt(const void* pData,size_t nSize,size_t offset)
{
    if(!pData||nSize==0) return 0;
    if(ReadableSize()==UnboundedBuffer::MAX_BUFFER_SIZE) return 0;

    AssureSpace_(nSize+offset);
    assert(nSize+offset<=WritableSize());

    memcpy(&buffer_[writePos_+offset],pData,nSize);
    return nSize;
}

size_t UnboundedBuffer::PushData(const void* pData,size_t nSize)
{
    size_t nBytes=PushDataAt(pData,nSize);
    AdjustWritePtr(nBytes);
    return nBytes;
}

size_t UnboundedBuffer::Write(const void* pData,size_t nSize) { return PushData(pData,nSize);}

size_t UnboundedBuffer::PeekDataAt(void* pBuf,size_t nSize,size_t offset)
{
    const size_t dataSize=ReadableSize();
    if(!pBuf||nSize==0||dataSize<offset)
        return 0;
    
    if(nSize+offset>dataSize)
        nSize=dataSize-offset;

    memcpy(pBuf,&buffer_[readPos_+offset],nSize);
    return nSize;
}

size_t UnboundedBuffer::PeekData(void* pBuf,size_t nSize)
{
    size_t nBytes=PeekDataAt(pBuf,nSize);
    AdjustReadPtr(nBytes);
    return nBytes;
}

void UnboundedBuffer::Shrink(bool tight)
{
    assert(buffer_.capacity()==buffer_.size());
    if(buffer_.empty())
    {
        assert(readPos_==0);
        assert(writePos_==0);
        return ;
    }
    size_t oldCap=buffer_.size();
    size_t dataSize=ReadableSize();
    if(!tight&&dataSize>oldCap/2)
        return ;
    std::vector<char> tmp;
    tmp.reserve(dataSize);
    memcpy(&tmp[0],&buffer_[readPos_],dataSize);
    tmp.swap(buffer_);
    readPos_=0;
    writePos_=dataSize;
    std::cout<<oldCap<<" shrink to "<<buffer_.size()<<std::endl;
}

void UnboundedBuffer::Clear() { readPos_=writePos_=0;}

void UnboundedBuffer::Swap(UnboundedBuffer& buf)
{
    buffer_.swap(buf.buffer_);
    std::swap(readPos_,buf.readPos_);
    std::swap(writePos_,buf.writePos_);
}

}