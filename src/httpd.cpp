#include"httpd.h"

using namespace std;


Httpd::Httpd(int threads,int port,string dir):stop(false)
{
    this->threads = threads;
    this->port = port;
    this->dir = dir;
}


int Httpd::StartHttpd()
{
    this->stop = false;
    _initHttpd();
    this->stop = true;
    return 0;
}

int Httpd::StopHttpd()
{
    this->stop = true;
    return 0;
}

void Httpd::_initHttpd()
{
    int server_socket = _initSocket();
    if(server_socket == -1)
    {
        return;
    }
    //配置连接socket
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = -1;

    //创建线程池
    ThreadPool thread_pool(this->threads);

    //将_handle方法绑定到task
    function<void(int)> task = bind(&Httpd::_handle,this,placeholders::_1);
    int Mode = fcntl(client_socket,F_GETFL,0);
    fcntl(client_socket,F_SETFL,Mode|O_NONBLOCK);
    while(!stop)
    {
        client_socket = accept(server_socket,(struct sockaddr*)&client_addr,&client_addr_len);

        if(client_socket == -1)
        {
            cout<<_getTime(TIME_CST)<<"accept socket error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
            return;
        }
        thread_pool.enqueue(task,client_socket);
    }
}

int Httpd::_initSocket()
{
    int listen_fd = -1;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));

    //设置socket配置
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(this->port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //创建socket
    if((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        cout<<_getTime(TIME_CST)<<"create socket error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
        return -1;
    }

    //设置操作码防止结束占用端口
    int opt = 1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    //socket绑定
    if(bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
    {
        cout<<_getTime(TIME_CST)<<"bind socket error:"<<strerror(errno)<<" (error:"<<errno<<")"<<endl;
        return -1;
    }

    //监听队列
    if(listen(listen_fd,10) == -1)
    {
        cout<<_getTime(TIME_CST)<<"listen socket error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
        return -1;
    }

    cout<<_getTime(TIME_CST)<<"listen to port: "<<this->port<<endl<<endl;

    return listen_fd;
}

void Httpd::_handle(int socket)
{
    char recv_buf[0x1000];
    memset(recv_buf,0,sizeof(recv_buf));
    if(recv(socket,(char*)&recv_buf,sizeof(recv_buf),0) == -1)
    {
        return;
    }
    string request = recv_buf;
    _resolutionRequest(socket,request);
    close(socket);
}

void Httpd::_resolutionRequest(int socket,string request)
{
    string line;
    string info;
    stringstream ss;
    ss<<request;

    getline(ss,line);
    ss.str(line);
    //请求的方法路径
    string request_func;
    string request_path;

    ss>>request_func;
    ss>>request_path;

    request_path = "../" + this->dir + request_path;

    if(request_func == "GET")
    {
        _sendFile(socket,request_path);
    }
    else if(request_func == "POST")
    {
        //对前端不了解，基础太差，看着迷糊
        _sendError(socket,ERROR_500);
    }

}

void Httpd::_sendFile(int socket,string fileName)
{
    if(fileName[fileName.size()-1] == '/')
    {
        fileName+="index.html";
    }

    ifstream ifs;
    //判断文件类型
    size_t pos = fileName.find_last_of(".");
    if(pos>fileName.size())
    {
        _sendError(socket,ERROR_404);
        return;
    }
    string suffix = fileName.substr(pos+1,fileName.size()-pos);
    string context_type;
    if(suffix == "html" || suffix == "css" || suffix == "js" || suffix == "xml")
    {
        ifs.open(fileName,ios::in);
        if(!ifs.is_open())
        {
            _sendError(socket,ERROR_404);
            return;
        }
        context_type = "text/"+suffix;

    }
    else
    {
        ifs.open(fileName,ios::in|ios::binary);
        if(!ifs.is_open())
        {
            _sendError(socket,ERROR_404);
            return;
        }
        context_type = "image/"+suffix;
    }

    //获取大小
    ifs.seekg(0,ifs.end);
    //装载响应头    
    size_t content_length = ifs.tellg();
    //2G
    if(content_length>0x400000000)
    {
        _sendError(socket,ERROR_404);
        return;
    }

    if(content_length == 0)
    {
        cout<<_getTime(TIME_CST)<<"can not open file "<< fileName <<" error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
        return;
    }
    string state = "200 OK";

    struct ResponseHeader header(state,context_type,content_length);
    if(send(socket,header.MessageHeader.c_str(),header.MessageHeader.size(),0) == -1)
    {
        return;
    }
    ifs.seekg(0,ifs.beg);
    char buf[0x1000];
    while(!ifs.eof())
    {
        ifs.read(buf,sizeof(buf));
        if(send(socket,buf,sizeof(buf),0)<0)
        {
            return;
        }
    }
    cout<<_getTime(TIME_CST)<<"send "<<fileName<<endl;

}

void Httpd::_sendError(int socket,int status)
{
    
    string path = "../" + this->dir +"/";
    ifstream ifs;

    if(status == ERROR_404)
    {
        ifs.open(path+"404.html",ios::in);
    }
    else
    {
        ifs.open(path+"500.html",ios::in);
    }

    if(!ifs.is_open())
    {
        cout<<_getTime(TIME_CST)<<"can not open file (404.html or 500.html) "<< path <<" error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
        return;
    }
    //获取大小
    ifs.seekg(0,ifs.end);
    //装载响应头    
    size_t content_length = ifs.tellg();
    if(content_length == 0)
    {
        cout<<_getTime(TIME_CST)<<"can not open file (404.html or 500.html) "<< path<<" error:"<<strerror(errno)<<" ("<<errno<<")"<<endl;
        return;
    }
    string state = "404 Not Found";
    if(status == ERROR_500)
    {   
        state = "500 Internal Server Error";
    }

    string content_type = "text/html";
    struct ResponseHeader header(state,content_type,content_length);

    //发送响应头
    if(send(socket,header.MessageHeader.c_str(),header.MessageHeader.size(),0) == -1)
    {
        return;
    }

    //准备发送文件
    ifs.seekg(0,ifs.beg);
    char buf[0x800];
    while(!ifs.eof())
    {
        ifs.read(buf,sizeof(buf));
        if(send(socket,buf,sizeof(buf),0) == -1)
        {
            return;
        }
    }
    if(status == ERROR_404)
    {
        cout<<_getTime(TIME_CST)<<"send error 404.html"<<endl;
    }
    else
    {
        cout<<_getTime(TIME_CST)<<"send error 500.html"<<endl;
    }

}

string _getTime(int status)
{
    stringstream ss;
    string s;
    time_t t = time(nullptr);
    string result;
    if(status == TIME_GMT)
    {
        s = "%a, %d %b %Y %H:%M:%S GMT";
        ss << put_time(gmtime(&t),s.c_str());
        result = ss.str();
    }
    else
    {
        s = "%Y-%m-%d %Y %H:%M:%S";
        ss<<put_time(localtime(&t),s.c_str());
        result = "["+ss.str()+"] ";
    }
    return result;
}