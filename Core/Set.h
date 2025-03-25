#pragma once

#include<unordered_set>

#include"Helper.h"

namespace myredis
{

using Set=std::unordered_set<String,
                my_hash,
                std::equal_to<String>>;
    
size_t SScanKey(const Set& set,size_t cursor,size_t cnt,std::vector<String>& ans);

}