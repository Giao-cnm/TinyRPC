#include"rocket/net/tcp/tcp_client.h"
#include"rocket/common/log.h"
#include<sys/socket.h>
#include<string.h>
#include"rocket/net/eventloop.h"
#include"rocket/net/fd_event_group.h"
namespace rocket {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr):m_peer_addr(peer_addr)
{
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(),SOCK_STREAM,0);
    if(m_fd < 0)
    {
        ERRORLOG("create socket error");
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();
    m_connection = std::make_shared<TcpConnection>(m_event_loop,m_fd,128,peer_addr,nullptr,TcpConnectionByClient);
    m_connection->setConnectionType(TcpConnectionByClient);
}
TcpClient::~TcpClient()
{
    DEBUGLOG("Rocket TcpClient::~TcpClient()");
    if(m_fd>0)
    {
        close(m_fd);
    }
}
void TcpClient::connect(std::function<void()> done)
{
    int rt = ::connect(m_fd,m_peer_addr->getSockAddr(),m_peer_addr->getSocklen());
    if(rt == 0)
    {
        DEBUGLOG("connect to %s success",m_peer_addr->toString().c_str());
        if(done)
        {
            done();
        }
    }else if(rt==-1){
       if(errno == EINPROGRESS)
       {
        m_fd_event->listen(FdEvent::OUT_EVENT,[this,done]()
        {
            int error = 0;
            socklen_t error_len = sizeof(error);
            getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);
            bool is_connect_succ = false;
            if(error == 0)
            {
                DEBUGLOG("connect [%s]success,",m_peer_addr->toString().c_str());
                is_connect_succ = true;
                m_connection->setState(Connected);
       
            }else{
                ERRORLOG("connect [%s]failed,errno:%d",m_peer_addr->toString().c_str(),error);
            }
            //去掉可写时间的触发
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
            //如果连接成功才执行回调函数
            if(is_connect_succ&& done)
            {
             
                done();
               
            }
        });
        m_event_loop->addEpollEvent(m_fd_event);
       if(!m_event_loop->isLooping())
       {
              m_event_loop->loop();
       }
       }else{
              ERRORLOG("connect [%s]failed,errno:%d",m_peer_addr->toString().c_str(),errno);
       }

    }
}
/*
1.异步的发送Message;
2.如果发送Message成功，会调用done回调函数，函数的入参就是Message对象;
*/
void TcpClient::writeMessage(AbstractProtocol::s_ptr message,std::function<void(AbstractProtocol::s_ptr)> done){
// 1，把Message对象写入到Connection 
    m_connection->pushSendMessage(message,done) ;
    m_connection->listenWrite();
}
/*
1.异步的读Message;
2.如果发送Message成功，会调用done回调函数，函数的入参就是Message对象;
*/
void TcpClient::readMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done){
//1.监听可读时间
//2.从buffer里decode得到Message对象,判断是否与req_id相等，相等则成功执行回调
    m_connection->pushReadMessage(req_id,done);
    m_connection->listenRead();

}

}
