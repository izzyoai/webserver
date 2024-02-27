# webserver

## 1.linux下c++ webserver

### 项目亮点

- 使用线程池来解决任务队列
- 读取config.conf文件来配置端口，线程数，以及webdir

### 一些不足和遗憾

- 没有用太多时间，比较仓促的完成

- 模块还是没有分得太清楚，都集中在httpd中

- 对前端不熟悉，只写了get请求，不过普通的资源请求还是完成了

- 刚开始对于send和recv方法的使用过程中，遇到一点问题就直接停止运行了，后面利用了等于-1这一判断来解决

- 刚开始运行结束后，端口会被占用，后面才清楚

  ```c++
      //设置操作码防止结束占用端口
      int opt = 1;
      setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
  ```

  

## 2.使用教程

### 1.使用cmake构建的项目

```
mkdir build
cd build
cmake ..
make


../bin/server
```

config和web_dir目录都在程序的../目录下

