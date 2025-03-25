#pragma once

#include<cstring>
#include<vector>

namespace myredis
{

class UnboundedBuffer
{
private:
    void AssureSpace_(size_t size);
    size_t readPos_;
    size_t writePos_;
    std::vector<char> buffer_;

public:
    static const size_t MAX_BUFFER_SIZE;

    UnboundedBuffer(): readPos_(0),writePos_(0) {}

    size_t PushDataAt(const void* pData,size_t nSize,size_t offset=0);
    size_t PushData(const void* pData,size_t nSize);
    size_t Write(const void* pData,size_t nSize);
    size_t PeekDataAt(void* pBuf,size_t nSize,size_t offset=0);
    size_t PeekData(void* pBuf,size_t nSize);

    void AdjustWritePtr(size_t nBytes) { writePos_+=nBytes;}
    void AdjustReadPtr(size_t nBytes) {readPos_+=nBytes;}

    char* ReadAddr() { return &buffer_[readPos_];}
    char* WriteAddr() { return &buffer_[writePos_];}

    bool IsEmpty() const { return ReadableSize()==0;}

    size_t ReadableSize() const { return writePos_-readPos_;}
    size_t WritableSize() const { return buffer_.size()-writePos_;}

    void Shrink(bool tight=0);
    void Clear();
    void Swap(UnboundedBuffer& buf);
};

}