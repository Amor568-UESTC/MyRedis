#pragma once

#include<list>

#include"String.h"

namespace myredis
{

enum class ListPosition
{
    head,
    tail,
};

using List=std::list<String>;

}