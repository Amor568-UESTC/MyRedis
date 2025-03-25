#pragma once

#include<stdint.h>

#include"String.h"

namespace myredis
{

struct Object;

class DumpInterface
{
public:
    virtual ~DumpInterface() {}
    virtual Object Get(const String& key)=0;
    virtual bool Put(const String& key,const Object& obj,int64_t ttl=0)=0;
    virtual bool Put(const String& key)=0;
    virtual bool Delete(const String& key)=0;
};

}