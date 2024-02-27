#include"config.h"
#include"httpd.h"

#include<iostream>
#include<map>

using namespace std;

int main()
{
    Config con("../config/config.conf");
    map<string,string> config;
    if(con.Is_result())
    {
        con.TransferConfig(config);
    }
    if(config.empty())
    {
        cout<<"配置为空"<<endl;
        return -1;
    }

    auto conf =config.find("SERVER_THREAD");
    string buf = conf->second;
    int server_thread = 0;
    for(size_t i = 0;i < buf.size();i++)
    { 
        server_thread=server_thread*10 + (buf[i] - '0');
    }

    conf = config.find("SERVER_PORT");
    buf = conf->second;
    int server_port = 0;
    for(size_t i = 0;i < buf.size();i++)
    {
        server_port=server_port*10 + (buf[i] - '0');
    }
    
    conf = config.find("SERVER_DIR");


    Httpd httpd(server_thread,server_port,conf->second);
    httpd.StartHttpd();
    
    return 0;
}