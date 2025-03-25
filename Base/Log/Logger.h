#pragma once

#include<string>
#include<set>
#include<memory>

#include"../Threads/ThreadPool.h"
#include"../AsyncBuffer.h"
#include"MemoryFile.h"
#include"../Timer.h"

enum LogLevel
{
    logINFO=0x01<<0,

};

enum LogDest
{
    logConsole=0x01<<0,
    logFILE=0x01<<1,

}

