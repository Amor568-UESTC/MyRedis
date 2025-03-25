#include<cassert>

#include"String.h"
#include"Store.h"
#include"Log/Logger.h"

namespace myredis
{

Object Object::CreateString(const String& value)
{
    Object obj(Type_string);

    long val;
    if(Strtol(value.c_str(),value.size(),&val))
    {
        obj.encoding=Encode_int;
        obj.value=(void*)val;
        DBG<<"set long value"<<val;
    }
    else 
    {
        obj.encoding=Encode_raw;
        obj.value=new String(value);
    }
    return obj;
}

Object Object::CreateString(long val)
{
    Object obj(Type_string);
    obj.encoding=Encode_int;
    obj.value=(void*)val;
    return obj;
}

static void DeleteString(String* s)
{ delete s; }

static void NotDeleteString(String* s) {}

std::unique_ptr<String,void (*)(String*)> 
    GetDecodedString(const Object* value)
{
    if(value->encoding==Encode_raw)
        return std::unique_ptr<String,void (*)(String*)>(value->CastString(),NotDeleteString);
    else if(value->encoding==Encode_int)
    {
        intptr_t val=(intptr_t)value->value;
        char vbuf[32];
        snprintf(vbuf,sizeof(vbuf)-1,"ld",val);
        return std::unique_ptr<String,void (*)(String*)>(new String(vbuf),DeleteString);
    }
    else 
        assert(!!!"error string encoding");

    return std::unique_ptr<String,void (*)(String*)>(nullptr,NotDeleteString);
}

static bool SetValue(const String& key,const String& value,bool exclusive=0)
{
    if(exclusive)
    {
        Object* val;
        if(STORE.GetValue(key,val)==Error_ok)
            return 0;
    }

    STORE.ClearExpire(key);
    STORE.SetValue(key,Object::CreateString(value));

    return 1;
}

Error set(const std::vector<String>& params,UnboundedBuffer* reply)
{
    if(params.size()%2!=1)
    {
        ReplyError(Error_param,reply);
        return Error_param;
    }

    for(size_t i=1;i<params.size();i+=2)
    {
        g_dirtyKeys.push_back(params[i]);
        SetValue(params[i],params[i+1]);
    }

    FormatOK(reply);
    return Error_ok;
}

Error msetnx(const std::vector<String>& params,UnboundedBuffer* reply)
{
    if(params.size()%2!=1)
    {
        ReplyError(Error_param,reply);
        return Error_param;
    }

    for(size_t i=1;i<params.size();i+=2)
    {
        Object* val;
        if(STORE.GetValue(params[i],val)==Error_ok)
        {
            Format0(reply);
            return Error_ok;
        }
    }

    for(size_t i=1;i<params.size();i+=2)
    {
        g_dirtyKeys.push_back(params[i]);
        SetValue(params[i],params[i+1]);
    }

    Format1(reply);
    return Error_ok;
}

Error setex(const std::vector<String>& params,UnboundedBuffer* reply)
{
    long seconds;
    if(!Strtol(params[2].c_str(),params[2].size(),&seconds))
    {
        ReplyError(Error_nan,reply);
        return Error_nan;
    }

    const auto& key=params[1];
    STORE.SetValue(key,Object::CreateString(params[3]));
    STORE.SetExpire(key,::Now()+seconds*1000);

    FormatOK(reply);
    return Error_ok;
}

Error psetex(const std::vector<String>& params,UnboundedBuffer* reply)
{
    long milliseconds;
    if(!Strtol(params[2].c_str,params[2].size(),&milliseconds))
    {
        ReplyError(Error_nan,reply);
        return Error_nan;
    }

    const auto& key=params[1];
    STORE.SetValue(key,Object::CreateString(params[3]));
    STORE.SetExpire(key,::Now()+milliseconds);

    FormatOK(reply);
    return Error_ok;
}

Error setrange(const std::vector<String>& params,UnboundedBuffer* reply)
{
    long offset;
    if(!Strtol(params[2].c_str(),params[2].size(),&offset))
    {
        ReplyError(Error_nan,reply);
        return Error_nan;
    }

    Object* value;
    Error err=STORE.GetValueByType(params[1],value,Type_string);
    if(err!=Error_ok)
    {
        if(err==Error_notExist)
            value=STORE.SetValue(params[1],Object::CreateString(""));
        else 
        {
            ReplyError(err,reply);
            return err;
        }
    }

    auto str=GetDecodedString(value);
    const size_t newSize=offset+params[3].size();

    if(newSize>str->size()) str->resize(newSize,'\0');
    str->replace(offset,params[3].size(),params[3]);

    if(value->encoding==Encode_int)
    {
        value->Reset(new String(*str));
        value->encoding=Encode_raw;
    }

    FormatInt(static_cast<long>(str->size()),reply);
    return Error_ok;
}

static void AddReply(Object* value,UnboundedBuffer* reply)
{
    auto str=GetDecodedString(value);
    FormatBulk(str->c_str(),str->size(),reply);
}

Error get(const std::vector<String>& params,UnboundedBuffer* reply)
{
    Object* value;
    Error err=STORE.GetValueByType(params[1],value,Type_string);
    if(err!=Error_ok)
    {
        if(err==Error_notExist)
            FormatNull(reply);
        else 
            ReplyError(err,reply);
        
        return err;
    }

    AddReply(value,reply);
    return Error_ok;
}

Error mget(const std::vector<String>& params,UnboundedBuffer* reply)
{
    PreFormatMultiBulk(params.size()-1,reply);
    for(size_t i=1;i<params.size();i++)
    {
        Object* value;
        Error err=STORE.GetValueByType(params[i],value,Type_string);
        if(err!=Error_ok)
            FormatNull(reply);
        else 
            AddReply(value,reply);
    }

    return Error_ok;
}

Error getrange(const std::vector<String>& params,UnboundedBuffer* reply)
{
    Object* value;
    Error err=STORE.GetValueByType(params[1],value,Type_string);
    if(err!=Error_ok)
    {
        if(err==Error_notExist)
            FormatBulk("",0,reply);
        else    
            ReplyError(err,reply);

        return err;
    }

    long start=0,end=0;
    if(!Strtol(params[2].c_str(),params[2].size(),&start)||
       !Strtol(params[3].c_str(),params[3].size(),&end))
    {
        ReplyError(Error_nan,reply);
        return Error_nan;
    }

    auto str=GetDecodedString(value);
    AdjustIndex(start,end,str->size());

    if(start<=end)
        FormatBulk(&(*str)[start],end-start+1,reply);
    else 
        FormatEmptyBulk(reply);
    
    return Error_ok;
}

Error getset(const std::vector<String>& params,UnboundedBuffer* reply)
{
    Object* value=nullptr;
    Error err=STORE.GetValueByType(params[1],value,Type_string);

    switch (err)
    {
    case Error_notExist:
    case Error_ok:
        if(!value)
            FormatNull(reply);
        else 
            FormatBulk(*GetDecodedString(value),reply);
            
        STORE.SetValue(params[1],Object::CreateString(params[2]));
        break;
    default:
        ReplyError(err,reply);
        break;
    }

    return Error_ok;
}

Error append(const std::vector<String>& params,UnboundedBuffer* reply)
{
    Object* value;
    Error err
}

}
