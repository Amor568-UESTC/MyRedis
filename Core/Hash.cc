#include<cassert>

#include"Hash.h"
#include"Store.h"

namespace myredis
{

Object Object::CreateHash()
{
    Object obj(Type_hash);
    obj.Reset(new Hash);
    return obj;
}

#define GET_HASH(hashname) \
    Object* value;  \
    Error err = STORE

}