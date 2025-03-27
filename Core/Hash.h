#pragma once

#include<unordered_map>

#include"String.h"
#include"Helper.h"

namespace myredis
{

using Hash=std::unordered_map<String,String,my_hash,std::equal_to<String>>;

size_t HScanKey(const Hash& hash,size_t cursor,size_t cnt,std::vector<String>& ans);
}