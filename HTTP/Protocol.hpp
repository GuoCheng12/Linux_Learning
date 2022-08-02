#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <unordered_map>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
#include "Util.hpp"
#include "Log.hpp"

#define SEP ": " /*  This is the separator used to the http request  */
#define OK 200 /* This is the status code used in the response part  */
#define NOT_FOUND 404  
class HttpRequest{
  public:
    std::string request_line;
    std::vector<std::string> request_hearder;
    std::string blank;
    std::string request_body;

    /*  after Parse  */
    std::string method;
    std::string uri;
    std::string version;

    std::unordered_map<std::string,std::string> header_kv;
    int content_length;
  public:
    HttpRequest():content_length(0) {};
    ~HttpRequest() {};

};

class HttpResponse{
  public:
    std::string status_line;
    std::vector<std::string> response_header;
    std::string blank;
    std::string response_body;

    int status_code;
  public:
    HttpResponse():status_code(OK){};
    ~HttpResponse() {};
};

class EndPoint{
  private:
    int sock;
    HttpRequest http_request;
    HttpResponse http_response;
  private:
    void RecvHttpRequestLine()
    {
      auto& line = http_request.request_line;
      Util::ReadLine(sock,line);
      line.resize(line.size()-1);
      LOG(INFO,http_request.request_line);
    }
    void RecvHttpRequesetHeader()
    {
      std::string line;
      while(true)
      {
        line.clear();
        Util::ReadLine(sock,line);
        if(line == "\n")
        {
          http_request.blank = line;
          break;
        }
        line.resize(line.size()-1);
        http_request.request_hearder.push_back(line); 
      }
    }
    void ParseHttpRequestLine()
    {
      auto& line = http_request.request_line;
      std::stringstream ss(line);
      ss >> http_request.method >> http_request.uri >> http_request.version;
      LOG(INFO,http_request.method);
      LOG(INFO,http_request.uri);
      LOG(INFO,http_request.version);
    }
    /*  Is there a body part that needs to be read:
     *  use Method == POST
     *
     *  Make sure that how many bytes we should be reading:
     *  'Content-Length' decide */
    void ParseHttpRequestHeader()
    {
      std::string key;
      std::string value;
      for(auto &iter : http_request.request_hearder)
      {
        if(Util::CutString(iter,key,value,SEP))
        {
          http_request.header_kv.insert({key,value});
        }
      }
    }
    /* Receive Body */
    bool IsNeedRecvHttpRequestBody()
    {
      auto method = http_request.method;
      if(method == "POST")
      {
        auto &header_kv = http_request.header_kv;
        auto iter = header_kv.find("Content-Length");
        if(iter != header_kv.end())
        {
          http_request.content_length = atoi(iter->second.c_str());
          return true;
        }
      }
      return false;
    }
    void RecvHttpRequestBody()
    {
      if(IsNeedRecvHttpRequestBody())
      {
        int content_length = http_request.content_length;
        auto &body = http_request.request_body;
        char ch = 0;
        while(content_length)
        {
          ssize_t s = recv(sock,&ch,1,0);  
          if(s > 0)
          {
            body.push_back(ch);
            content_length--;
          }
          else
          {
            break;
          }
        }
      }
    }
  public:
    EndPoint(int _sock)
    :sock(_sock)
    {}
    void RecvHttpRequest()
    {
      RecvHttpRequestLine();
      RecvHttpRequesetHeader();
      ParseHttpRequestLine();
      ParseHttpRequestHeader();
      IsNeedRecvHttpRequestBody();
    }
    void BuildHttpReponse()
    {
      if(http_request.method != "GET" || http_request.method != "POST")
      {
        /* illegal request */
        
      }
    }

    void SendHttpReponse()
    {}

    ~EndPoint()
    {
      close(sock);
    }
};
class Entrance{
  public:
    static void *HandlerRequest(void *_sock)
    {
      LOG(INFO,"Hander Request Begin:..\n"); 
      int sock = *(int*)_sock;
      delete (int*)_sock;
#ifdef DEBUG
      char buffer[4096];
      recv(sock,buffer,sizeof(buffer),0);
      std::cout << "----------------begin---------------------" << '\n';
      std::cout << buffer << std::endl;
      std::cout <<"-----------------end-----------------------" << '\n';
#else
      EndPoint *ep = new EndPoint(sock);
      ep->RecvHttpRequest();
      ep->BuildHttpReponse();
      ep->SendHttpReponse();
#endif
      LOG(INFO,"Hander Request end..\n");
      return nullptr;
    }
};
