#ifndef ROCKET_NET_TCP_TCP_CLIENT_H
#define ROCKET_NET_TCP_TCP_CLIENT_H
#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_connection.h"
#include"rocket/net/eventloop.h"
#include"rocket/net/coder/abstract_protocol.h"
namespace rocket {

class TcpClient {
public:
    TcpClient(NetAddr::s_ptr peer_addr);
    
    ~TcpClient();

    void connect(std::function<void()> done);
    
    void writeMessage(AbstractProtocol::s_ptr request,std::function<void(AbstractProtocol::s_ptr)> done);

    void readMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)> done);
    
    void disconnect();
    
    void send();
    
    void receive();
private:
    NetAddr::s_ptr m_peer_addr;

    TcpConnection::s_ptr m_connection;
    
    EventLoop* m_event_loop;
    
    int m_fd{-1};
    
    FdEvent* m_fd_event {NULL};
    
  



};

}

#endif