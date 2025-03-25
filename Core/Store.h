#pragma once

#include<vector>
#include<map>
#include<memory>

#include"Common.h"
#include"Set.h"
#include"SortedSet.h"
#include"Hash.h"
#include"List.h"
#include"Timer.h"
#include"DumpInterface.h"

namespace myredis
{

using PSTRING=String*;
using PLIST=List*;
using PSET=Set*;
using PSSET=SortedSet*;
using PHASH=Hash*;

static const int kLRUBits=24;
static const uint32_t kMaxLRUValue=(1<<kLRUBits)-1;

uint32_t EstimateIdleTime(uint32_t lru);

struct Object
{
private:
    void MoveFrom_(Object&& obj);
    void FreeValue_();

public:
    static uint32_t lruclock;

    unsigned int type:4;
    unsigned int encoding:4;
    unsigned int lru:kLRUBits;

    void* value;

    explicit 
    Object(Type=Type_invalid);
    ~Object();

    Object(const Object& obj)=delete;
    Object& operator=(cosnt Object& obj)=delete;

    Object(Object&& obj);
    Object& operator=(Object&& obj);

    void Clear();
    void Reset(void* newvalue=nullptr);

    static Object CreateString(const String& value);
    static Object CreateString(long value);
    static Object CreateList();
    static Object CreateSet();
    static Object CreateSSet();
    static Object CreateHash();

    PSTRING CastString const { return reinterpret_cast<PSTRING>(value);}
    PSTRING CastList const { return reinterpret_cast<PLIST>(value);}
    PSTRING CastSet const { return reinterpret_cast<PSET>(value);}
    PSTRING CastSSet const { return reinterpret_cast<PSSET>(value);}
    PSTRING CastHash const { return reinterpret_cast<PHASH>(value);}
};

class Client;

using DB=std::unordered_map<String,Object,my_hash,std::equal_to<String>>;

const int kMaxDbNum=65536;

class Store
{
private:
    Store(): dbno_(0) {}
    
}

}