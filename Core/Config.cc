#include<vector>
#include<iostream>

#include"Config.h"
#include"ConfigParser.h"

namespace myredis
{

static void EraseQuotes(String& str)
{
    if(str.size()<2)
        return ;
    if(str[0]=='"'&&str.back()=='"')
    {
        str.erase(str.begin());
        str.pop_back();
    }
}

extern std::vector<String> SplitString(const String& str,char seperator);

Config g_config;

Config::Config()
{
    daemonize=0;
    pidFile="/var/run/myredis.pid";

    ip="127.0.0.1";
    port=6379;
    timeout=0;

    logLevel="notice";
    logDir="stdout";

    databases=16;

    //rdb
    saveSeconds=999999999;
    saveChanges=999999999;
    RdbCompression=1;
    RdbCheckSum=1;
    RdbFullName="./dump.rdb";
    maxClients=10000;

    //aof
    appendOnly=0;
    appendFilename="appendonly.aof";
    appendFsync=0;

    //slow log
    slowLogTime=0;
    slowLogMaxLen=128;
    hz=10;
    includeFile="";

    maxMemory=2*1024*1024UL;
    maxMemorySamples=5;
    noeviction=1;

    backEnd=BackEndNode;
    backEndPath="dump";
    backEndHz=10;
}

bool LoadRedisConfig(const char* cfgFile,Config& cfg)
{
    ConfigParser parser;
    if(!parser.Load(cfgFile))
        return 0;

    if(parser.GetData<String>("daemonize")=="yes")
        cfg.daemonize=1;
    else 
        cfg.daemonize=0;

    cfg.pidFile=parser.GetData<String>("pidFile",cfg.pidFile);
    cfg.ip=parser.GetData<String>("bind",cfg.ip);
    cfg.port=parser.GetData<unsigned short>("port");
    cfg.timeout=parser.GetData<int>("timeout");
    cfg.logLevel=parser.GetData<String>("logLevel",cfg.logLevel);
    cfg.logDir=parser.GetData<String>("logFile",cfg.logDir);

    EraseQuotes(cfg.logDir);
    if(cfg.logDir.empty())
        cfg.logDir="stdout";

    cfg.databases=parser.GetData<int>("databases",cfg.databases);
    cfg.password=parser.GetData<String>("requirepass");
    EraseQuotes(cfg.password);

    //alias command
    std::vector<String> alias(SplitString(parsser.GetData<String>("rename-command"),' '));
    if(alias.size()%2==0)
        for(auto it(alias.begin());it!=alias.end();)
        {
            const String& oldCmd=*(it++);
            const String& newCmd=*(it++);
            cfg.aliases[oldCmd]=newCmd;
        }

    //load rdb config
    std::vector<String> saveInfo(SplitString(parser.GetData<String>("save"),' '));
    if(!saveInfo.empty()&&saveInfo.size()!=2)
    {
        EraseQuotes(saveInfo[0]);
        if(!(saveInfo.size()==1&&saveInfo[0].empty()))
        {
            std::cerr<<"bad format save rdb interval,bad string"
                     <<parser.GetData<String>("save")
                     <<std::endl;
            return 0;
        }
    }
    else if(!saveInfo.empty())
    {
        cfg.saveSeconds=std::stoi(saveInfo[0]);
        cfg.saveChanges=std::stoi(saveInfo[1]);
    }

    if(cfg.saveSeconds==0) cfg.saveSeconds=999999999;
    if(cfg.saveChanges==0) cfg.saveChanges=999999999;

    cfg.RdbCompression=(parser.GetData<String>("RdbCompression")=="yes");
    cfg.RdbCheckSum=(parser.GetData<String>("RdbCheckSum")=="yes");
    cfg.RdbFullName=parser.GetData<String>("dir","./")+
                    parser.GetData<String>("dbfilename","dump.rdb");
    cfg.maxClients=parser.GetData<int>("maxVlients",10000);
    
    cfg.appendOnly(parser.GetData<String>("appendOnly","no")=="yes");
    cfg.appendFilename=parser.GetData<const char*>("appendFileName","appendOnly.aof");
    if(cfg.appendFilename.size()<=2)
        return 0;
    
    if(cfg.appendFilename[0]=='"')
        cfg.appendFilename=cfg.appendFilename.substr(1,cfg.appendFilename.size()-2);
    
    String tmpFsync=parser.GetData<const char*>("appendFsync","no");

    cfg.slowLogTime=parser.GetData<int>("slowlog-log-slower-than",0);
    cfg.slowLogMaxLen=parser.GetData<int>("slowlog-max-len",cfg.slowLogMaxLen);
    cfg.hz=parser.GetData<int>("hz",10);

    //load master ip port
    std::vector<String> master(SplitString(parser.GetData<String>("slaveof"),' '));
    if(master.size()==2)
    {
        cfg.masterIp=std::move(master[0]);
        cfg.masterPort=static_cast<unsigned short>(std::stoi(master[1]));
    }
    cfg.masterAuth=parser.GetData<String>("masterAuth");
    cfg.modules=parser.GetData<String>("loadmodule");
    cfg.includeFile=parser.GetData<String>("include");

    cfg.maxMemory=parser.GetData<uint64_t>("max")
    
    
}

}