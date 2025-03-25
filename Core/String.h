#pragma once

#include<string>
#include<memory>

namespace myredis
{
using String=std::string;

struct Object;

std::unique_ptr<String,void (*)(String*)>
GetDecodedString(const Object* value);

}