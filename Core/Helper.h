#pragma once

#include<vector>
#include<cstdlib>

#include"String.h"

namespace myredis
{

extern unsigned int dictGenHashFunction(const void* key,int len);

struct my_hash
{
    size_t operator() (const String& str) const;
};

size_t BitCnt(const uint8_t* buf,size_t len);

template<typename HASH>
inline typename HASH::const_local_iterator RandomHashMember(const HASH& container)
{
    if(container.empty())
        return typename HASH::const_local_iterator();

    while (1)
    {
        size_t bucket=random()%container.bucket_count();
        if(container.bucket_size(bucket)==0)
            continue;
        
        long lucky=random()%container.bucket_size(bucket);
        typename HASH::const_local_iterator it=container.begin(bucket);
        while(lucky>0)
        {
            ++it;
            --lucky;
        }
        return it;
    }
    return typename HASH::const_local_iterator();
}

template<typename HASH>
inline size_t ScanHashMember(const HASH& container,size_t cursor,size_t cnt,
                            std::vector<typename HASH::const_local_iterator>& ans)
{
    if(cursor>=container.size())
        return 0;
    auto idx=cursor;
    for(decltype(container.bucker_count())) bucket=0;bucket<container.bucket_count();bucket++)
    {
        const auto bktSize=container.bucket_size(bucket);
        if(idx<bktSize)
        {
            auto it=container.begin(bucket);
            while(idx>0)
            {
                ++it;
                --idx;
            }
            size_t newCursor=cursor;
            auto end=container.end(bucket);
            while(ans.size()<cnt&&it!=end)
            {
                ++newCursor;
                ans.push_back(it++);
                if(it==end)
                {
                    while(++bucket<container.bucket_count())
                        if(container.bucket_size(bucket)>0)
                        {
                            it=container.begin(bucket);
                            end=container.end(bucket);
                            break;
                        }
                    if(bucket==container.bucket_count())
                        return 0;
                }
            }
            return newCursor;
        }
        else 
            idx-=bktSize;
    }
    return 0;
}

extern void getRandomHexChars(char* p,unsigned int len);

enum MemoryInfoType
{
    VmPeak=0,
    VmSize=1,
    VmLck=2,
    VmHWM=3,
    VmRSS=4,
    VmSwap=5,
    VmMax=VmSwap+1,
};

extern std::vector<size_t> getMemoryInfo();
extern size_t getMemoryInfo(MemoryInfoType type);

}