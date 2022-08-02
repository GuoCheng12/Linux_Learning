#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
//工具类
class Util{
  public:
    static int ReadLine(int sock,std::string &out)
    {
      char ch = 'X';
      while(ch != '\n')
      {
        ssize_t s = recv(sock,&ch,1,0);
        if(s > 0)
        {
          if(ch == '\r')
          {
            //\r->\n or \r\n->\n 
            //MGE_PEER 窥探
            recv(sock,&ch,1,MSG_PEEK);
            if(ch == '\n')
            {
              //窥探成功
              recv(sock,&ch,1,0);
            }
            else
            {
              ch = '\n';
            }
          }
        }
        else if(s == 0)
        {
          return 0;
        }
        else
        {
          return -1;
        }
      }
      return out.size();
    }
    static bool CutString(const std::string &target,std::string key_out,std::string value_out,std::string sep)
    {
      size_t pos = target.find(sep);
      if(pos != std::string::npos)
      {
        key_out = target.substr(0,pos);
        value_out = target.substr(pos+sep.size());
        return true;
      }
      return false;
    }
};
