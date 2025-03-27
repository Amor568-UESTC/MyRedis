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

    unsigned int type:4;    //占用4位
    unsigned int encoding:4;
    unsigned int lru:kLRUBits;

    void* value;

    explicit
    Object(Type=Type_invalid);
    ~Object();

    Object(const Object& obj)=delete;
    Object& operator=(const Object& obj)=delete;

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
    Error GetValueByType_(const String& key,Object*& value,Type type=Type_invalid);
    ExpireResult ExpireIfNeed_(const String& key,uint64_t now);

    class ExpiresDB
    {
    private:
        using EXPIRE_DB=std::unordered_map<String,uint64_t,
                                            my_hash,
                                            std::equal_to<String>>;
        EXPIRE_DB expireKeys_;

    public:
        void SetExpire(const String& key,uint64_t when);
        int64_t TTL(const String& key,uint64_t now);
        bool ClearExpire(const String& key);
        ExpireResult ExpireIfNeed(const String& key,uint64_t now);

        int LoopCheck(uint64_t now);
    };

    class BlockedClients
    {
    private:
        using Clients=std::list<std::tuple<std::weak_ptr<Client>,uint64_t,ListPosition>>;
        using WaitingList=std::unordered_map<String,Clients>;
        WaitingList blockedClients_;

    public:
        bool BlockClient(const String& key,Client* client,uint64_t timeout,ListPosition pos,const String* dstList=0);
        size_t UnblockClient(Client* client);
        size_t ServeClient(const String& key,const PLIST& list);

        int LoopCheck(Client* client);
        size_t Size() const { return blockedClients_.size(); }
    };

    Error SetValue_(const String& key,Object& value,bool exclusive=0);

    mutable std::vector<DB> store_;
    mutable std::vector<ExpiresDB> expiresDB_;
    std::vector<BlockedClients> blockedClients_;
    std::vector<std::unique_ptr<DumpInterface>> backends_;

    using ToSyncDb=std::unordered_map<String,const Object*,
                                        my_hash,
                                        std::equal_to<String>>;
    std::vector<ToSyncDb> waitSyncKeys_;
    int dbno_;
};

#define STORE Store::Instance()

extern std::vector<String> g_dirtyKeys;
extern void Propogate(const std::vector<String>& params);
extern void Propogate(int dbno,const std::vector<String>& params);

}