#include<pthread.h>
#include<iostream>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include"rocket/common/log.h"
#include"rocket/common/config.h"
#include"rocket/net/fd_event.h"
#include"rocket/net/eventloop.h"
int main()
{
   
    rocket::Config::SetGlobalConfig("/home/mispalojar/code/TinyRpc-master/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    rocket::EventLoop* evnetloop = new rocket::EventLoop();
    std::cout<<"111"<<std::endl;

    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    
    if(listenfd == -1)
    {
        ERRORLOG("listenfd = -1");
        exit(0);
    }
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_port = htons(12311);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rt = bind(listenfd,reinterpret_cast<sockaddr*>(&addr),sizeof(addr));
    
    if(rt != 0)
    {
        ERRORLOG("Bind error!");
        exit(1);
    }
    
    rt = listen(listenfd,100);
    if(rt != 0)
    {
        ERRORLOG("listen error!");
        exit(1);
    }
    rocket::FdEvent event(listenfd);
    event.listen(rocket::FdEvent::IN_EVENT,[listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr,0,sizeof(peer_addr));
        int clientfd = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr),&addr_len);
        inet_ntoa(peer_addr.sin_addr);
        DEBUGLOG("success get client fd[%d],perr_addr: [%s %d]",clientfd,inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port)); 
    });
    evnetloop->addEpollEvent(&event);
    evnetloop->loop();
    return 0;
}