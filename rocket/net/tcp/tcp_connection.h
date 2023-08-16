#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H 
#include<memory>
#include<map>
#include<queue>
#include<set>
#include<functional>

#include"rocket/net/tcp/net_addr.h"
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/io_thread.h"
#include"rocket/net/coder/abstract_protocol.h"
#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/coder/tinypb_coder.h"
#include"rocket/net/rpc/rpc_dispatcher.h"

namespace rocket{

enum TcpState{
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4,
};

enum TcpConnectionType{
    TcpConnectionByServer = 1, //作为服务端使用代表一个客户端连接
    TcpConnectionByClient = 2,//作为客户端使用代表一个服务端连接
};

class TcpConnection{
public: 
    typedef  std::shared_ptr<TcpConnection> s_ptr;

public:
    TcpConnection(EventLoop* eventloop,int fd,int buffer_size,NetAddr::s_ptr peer_addr,NetAddr::s_ptr local_addr,TcpConnectionType type = TcpConnectionByServer);

    ~TcpConnection();

    void excute();

    void onRead();

    void onWrite();

    void setState(const TcpState state);

    TcpState getState();

    void clear();

    // int getFd();

    void shutdown();

    void setConnectionType(TcpConnectionType type);

    void listenWrite();

    void listenRead();

    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    void pushReadMessage(const std::string& req_id,std::function<void(AbstractProtocol::s_ptr)>done );
   
    NetAddr::s_ptr getLocalAddr();

    NetAddr::s_ptr getPeerAddr();

private:

    
    NetAddr::s_ptr m_local_addr;

    NetAddr::s_ptr m_peer_addr;

    TcpBuffer::s_ptr m_in_buffer;

    TcpBuffer::s_ptr m_out_buffer;



    FdEvent* m_fd_event{NULL};

    TcpState m_state;

    EventLoop* m_event_loop{NULL};

    int m_fd{0};
    TcpConnectionType m_connection_type {TcpConnectionByServer};
    
    
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;

    AbstractCoder* m_coder{NULL};
    
    std::map<std::string,std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;

    std::shared_ptr<RpcDispatcher> m_dispatcher;

    };
}


#endif