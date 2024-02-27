#ifndef HTTPD_H
#define HTTPD_H

#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>

#include<iostream>
#include<string>
#include<vector>
#include<chrono>
#include<sstream>
#include<iomanip>
#include<ctime>
#include<functional>
#include<fstream>


#include"threadpool.h"

enum status{
    ERROR_404 = 1,
    ERROR_500,
    TIME_GMT,
    TIME_CST
};

//获取时间GMT与CST
std::string _getTime(int status);

//响应头
struct ResponseHeader{
    std::string MessageHeader;
    //构造消息体 状态行 日期 返回类型 返回长度
    ResponseHeader(std::string state,std::string content_type,size_t content_length)
    {
        std::string date = _getTime(TIME_GMT);
        MessageHeader = "HTTP/1.1 " + state + "\r\nDate: " + date +"\r\nServer: IzzyoHttpd/1.1\r\nContent-Type: " 
        + content_type + "; charset=UTF-8\r\nContent-Length: " + std::to_string(content_length) +"\r\n\r\n";
    }   
};



class Httpd{
public:
    //线程数，端口号，web目录
    Httpd(int threads,int port,std::string dir);
    //开启服务
    int StartHttpd();
    //关闭服务
    int StopHttpd();

private:
    //初始化httpd
    void _initHttpd();
    //初始化socket
    int _initSocket();
    //处理handle
    void _handle(int socket);
    //解析请求
    void _resolutionRequest(int socket,std::string request);
    //发送文件
    void _sendFile(int socket,std::string fileName);
    //发送错误 404 or 500
    void _sendError(int socket,int status);
private:
    int port;
    int threads;
    std::string dir;
    bool stop;
};

#endif