#pragma once

#include<map>
#include<vector>

#include"String.h"

namespace myredis
{

enum BackEndType
{
    BackEndNone=0,
    BackEndLeveldb=1,
    BakcENdMax=2,
};

struct Config
{
    bool daemonize;
    String pidFile;

    String ip;
    unsigned short port;

    int timeout;

    String logLevel;
    String logDir;

    int databases;

    String password;

    std::map<String,String> aliases;

    //RDB
    int saveSeconds;
    int saveChanges;
    bool RdbCompression;
    bool RdbCheckSum;
    String RdbFullName;

    int maxClients;

    //AOF
    bool appendOnly;
    String appendFilename;
    int appendFsync;

    //SlowLog
    int slowLogTime;
    int slowLogMaxLen;

    int hz;

    String masterIp;
    unsigned short masterPort;
    String masterAuth;

    String runId;
    String includeFile;

    std::vector<String> modules;

    //cache
    uint64_t maxMemory;
    int maxMemorySamples;
    bool noeviction;

    int backEnd;
    String backEndPath;
    int backEndHz;

    //cluster
    bool enableCluster=0;
    std::vector<String> centers;
    int setId=-1;

    Config();

    bool CheckArgs() const;
    bool CHeckPassword(const String& pwd) const;
};

extern Config g_config;

extern bool LoadRedisConfig(const char* cfgFile,Config& cfg);


}