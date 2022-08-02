#pragma once

#include <iostream>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include "Log.hpp"

#define BACKLOG 5
class TcpServer{
  private:
    int port;
    int listen_sock;
    static TcpServer* svr;
  private:
    /*  单例  */
    TcpServer(int _port)
      :port(_port)
      ,listen_sock(-1)
      {}
    TcpServer(const TcpServer &s)
      {}

  public:
    static TcpServer *getinstance(int port)
    {
      static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
      if(nullptr == svr)
      {
        pthread_mutex_lock(&lock);
        if(nullptr == svr)
        {
          svr = new TcpServer(port);
          svr->InitServer();
        }
        pthread_mutex_unlock(&lock);
      }
      return svr;
    }
    void InitServer()
    {
      Socket();
      Bind();
      Listen();
      LOG(INFO,"tcp_server init...success!\n");
    }
    void Socket()
      /*  套接字  */
    {
      listen_sock = socket(AF_INET,SOCK_STREAM,0); //创建套接字socket
      if(listen_sock < 0) //套接字创建失败
      {
        LOG(FATAL,"socket error!\n"); 
        exit(1);  
      }
      int opt = 1;
      setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//端口复用
      LOG(INFO,"building socket successful !\n");
    }
    void Bind()
      /*   绑定     */
    {
      struct sockaddr_in local;
      memset(&local,0,sizeof(local));
      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      local.sin_addr.s_addr = INADDR_ANY; //云服务器不能直接绑定公网IP
      if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local)) < 0) //绑定失败
      {
        printf("errno is %d\n",errno);
        LOG(FATAL,"bind error!\n");
        std::cout << local.sin_addr.s_addr << std::endl;
        exit(2);
      }
      LOG(INFO,"Bind success!\n");
    }
    void Listen()
      /*  监听    */
    {
      if(listen(listen_sock,BACKLOG) < 0) //监听失败
      {
        LOG(FATAL,"listen socket error!\n");
        exit(3);
      }
        LOG(INFO,"Building listen socket success!\n");
    }
    int Sock()
    {
      return listen_sock;
    }
    ~TcpServer()
    {
      if(listen_sock >= 0)
      {
        close(listen_sock);
      }
    }

};
TcpServer* TcpServer::svr = nullptr;
