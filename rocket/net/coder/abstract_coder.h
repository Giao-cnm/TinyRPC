#ifndef ROCKET_NET_CODER_ABSTRACT_CODER_H
#define ROCKET_NET_CODER_ABSTRACT_CODER_H
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/net/coder/abstract_protocol.h"
#include<vector>
namespace rocket{


class AbstractCoder{
public:

  
    //将Message转换为二进制流
    virtual void encode(std::vector<AbstractProtocol::s_ptr>&messages,TcpBuffer::s_ptr out_buffer) = 0;
    virtual void decode(std::vector<AbstractProtocol::s_ptr>&out_messages,TcpBuffer::s_ptr buffer) = 0;
    virtual ~AbstractCoder(){};
};


}


#endif