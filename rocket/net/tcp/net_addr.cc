#include<string.h>
#include<iostream>
#include"./net_addr.h"
#include"rocket/common/log.h"

namespace rocket{

    bool IPNetAddr::CheckVaild(const std::string& addr)
    {
        size_t i = addr.find_first_of(":");
        if(i == addr.npos)
        {
            return false;
        }
        std::string ip = addr.substr(0,i);
        std::string port = addr.substr(i+1,addr.size()-i-1);
        if(ip.empty()|| port.empty())
        {
            return false;
        }
        int iport = std::atoi(port.c_str());
        if(iport<= 0|| iport> 65536)
        {
            return false;
        }
        return true;
    }

    IPNetAddr::IPNetAddr(const std::string& ip,uint16_t port):m_ip(ip),m_port(port) //ip:port格式
    {
        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }

    IPNetAddr::IPNetAddr(const std::string&addr)// xxx.xxx.xxx.xxx:xxxx格式
    {
        size_t i = addr.find_first_of(":");
        if(i ==addr.npos)
        {
            ERRORLOG("invalid ipv4 addr %s", addr.c_str());
            return;
        }
        m_ip = addr.substr(0,i);
        m_port = std::atoi(addr.substr(i+1,addr.size()-1-i).c_str());

        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
        m_addr.sin_port = htons(m_port);
    }

    IPNetAddr::IPNetAddr(sockaddr_in addr):m_addr(addr) //sockaddr_in格式
    {
        m_ip = std::string(inet_ntoa(m_addr.sin_addr));
        m_port = ntohs(m_addr.sin_port);
    }

    sockaddr* IPNetAddr::getSockAddr()
    {
        return reinterpret_cast<sockaddr*> (&m_addr);
    }

    int  IPNetAddr::getFamily()
    {
          
        return AF_INET;
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         

    std::string IPNetAddr::toString()
    {
        std::string res;
        res = m_ip + ":" + std::to_string(m_port);
        return res;
    }
    socklen_t IPNetAddr::getSocklen() {
        return sizeof(m_addr);
            }

    bool IPNetAddr::CheckValid()
    {
        if(m_ip.empty())
        {
            return false;
        }

        if(m_port<0 || m_port> 65536)
        {
            return false;
        }

        if(inet_addr(m_ip.c_str())==INADDR_NONE)
        {
            return false;
        }
        return true;
    }


}