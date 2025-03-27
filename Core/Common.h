#pragma once

#include<cstddef>
#include<stdio.h>
#include<vector>
#include<algorithm>
#include<functional>
#include<strings.h>

#include"String.h"

#define 
#define CRLF "\r\n"

namespace myredis
{

const int kStringMaxBytes=1*1024*1024*1024;

enum Type
{
    Type_invalid,
    Type_string,
    Type_list,
    Type_set,
    Type_sortedSet,
    Type_hash,
};

enum Encode
{
    Encode_invalid,
    Encode_raw,
    Encode_int,
    Encode_list,
    Encode_set,
    Encode_hash,
    Encode_sset,
};

inline const char* EncodingStringInfo(unsigned encode)
{
    switch (encode)
    {
    case Encode_raw:
        return "raw";
    case Encode_int:
        return "int";
    case Encode_list:
        return "list";
    case Encode_set:
        return "set";
    case Encode_hash:
        return "hash";
    case Encode_sset:
        return "sset";
    default:
        break;
    }
    return "unknown";
}

enum Error
{
    Error_nop=-1,
    Error_ok=0,
    Error_type=1,
    Error_exist=2,
    Error_noExist=3,
    Error_param=4,
    Error_unknowCmd=5,
    Error_nan=6,
    Error_syntax=7,
    Error_dirtyExec=8,
    Error_watch=9,
    Error_noMulti=10,
    Error_invalidDB=11,
    Error_redonlySlave=12,
    Error_needAuth=13,
    Error_errAuth=14,
    Error_nomodule=15,
    Error_moduleinit=16,
    Error_modulunint=17,
    Error_modulerepeat=18,
    Error_busykey=19,
    Error_max,
};

extern struct ErrorInfo
{
    int len;
    const char* errorStr;
}g_errorInfo[];

template<typename T>
inline size_t Number2Str(char* ptr,size_t nBytes,T val)
{
    if(!ptr||nBytes<2) return 0;

    if(val==0)
    {
        ptr[0]='0';
        ptr[1]=0;
        return 1;
    }

    bool negative=0;
    if(val<0)
    {
        negative=1;
        val=-val;
    }

    size_t off=0;
    while(val>0)
    {
        if(off>=nBytes) return 0;
        ptr[off++]=val%10+'0';
        val/=10;
    }

    if(negative)
    {
        if(off>=nBytes) return 0;
        ptr[off++]='-';
    }

    std::reverse(ptr,ptr+off);
    ptr[off]=0;

    return off;
}

class UnboundedBuffer;

size_t FormatInt(long value,UnboundedBuffer* reply);
size_t FormatSingle(const char* str,size_t len,UnboundedBuffer* reply);
size_t FormatSingle(const String& str, UnboundedBuffer* reply);
size_t FormatBulk(const char* str,size_t len,UnboundedBuffer* reply);
size_t FormatBulk(const QString& str,UnboundedBuffer* reply);
size_t PreFormatMultiBulk(size_t nBulk,UnboundedBuffer* reply);
size_t FormatMultiBulk(const vector<String> vs,UnboundedBuffer* reply);
size_t FormatEmptyBulk(UnboundedBuffer* reply);
size_t FormatNull(UnboundedBuffer* reply);
size_t FormatNullArray(UnboundedBuffer* reply);
size_t FormatOK(UnboundedBuffer* reply);
size_t Format1(UnboundedBuffer* reply);
size_t Format0(UnboundedBuffer* reply);

void ReplyError(Error err,UnboundedBuffer* reply);

inline void AdjustIndex(long& start,long& end,size_t size)
{
    if(size==0)
    {
        end=0,start=1;
        return ;
    }

    if(start<0) start+=size;
    if(start<0) start=0;
    if(end<0) end+=size;
    if(end>=static_cast<long>(size)) end=size-1;
}

struct NocaseComp
{
    bool operator() (const String& s1,const String& s2) const
    { return strcasecmp(s1.c_str(),s2.c_str())<0; }

    bool operator() (const char* s1,const String& s2) const
    { return strcasecmp(s1,s2.c_str())<0; }

    bool operator() (const String& s1,const char* s2) const
    { return strcasecmp(s1.c_str(),s2)<0; }
};

enum class ParseResult:int8_t
{
    ok,
    wait,
    error,
};

ParseResult GetIntUntilCRLF(cosnt char*& ptr,size_t nBytes,int& val)

std::vector<String> SplitString(const String& str,char seperator);

template<typename ...Args>
std::string BuildInlineRequest(Args&& ...);

template<typename ...Args>
inline std::string BuildInlineRequest(S&& s)
{ return std::string(std::forward<S>(s))+CRLF; }

template<typenmae H,typename... T>
inline std::string BuildInlineRequest(H&& head,T&&... tails)
{
    std::string h(std::forward<H>(head));
    return h+" "+BuildInlineRequest(std::forward<T>(tails)...);
}

class ExecuteOnScopeExit
{
private:
    std::function<void ()> func_;

public: 
    ExecuteOnScopeExit() {}
    ExecuteOnScopeExit(ExcuteOnScopeExit&& e)
    { func_=std::move(e.func_); }

    ExecuteOnScopeExit(const ExecuteOnScopeExit& e)=delete;
    void operator=(const ExecuteOnScopeExit& f)=delete;

    template<typename F,typename... Args>
    ExecuteOnScopeExit(F&& f,Args&&... args)
    {
        auto tmp=std::bind(std::forward<F>(f),std::forward<Args>(args)...);
        func_=[tmp]() { (void)temp(); };
    }

    ~ExecuteOnScopeExit() nocexcept
    { if(func_) func_(); }
};

#define CONCAT(a,b) a##b
#define _MAKE_DEFER_HELPER_(line) myredis::ExecuteOnScopeExit CONCAT(defer,line)=[&]()

#define MYREDIS_DEFER_MAKE_DEFER_HELPER_(_LINE_)

}