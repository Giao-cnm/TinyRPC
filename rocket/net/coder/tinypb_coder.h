#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H
#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/coder/tinypb_protocol.h"
namespace rocket {

class TinyPbCoder : public AbstractCoder
{
    public:
    TinyPbCoder(){};
    void encode(std::vector<AbstractProtocol::s_ptr>&messages,TcpBuffer::s_ptr out_buffer);
    void decode(std::vector<AbstractProtocol::s_ptr>&out_messages,TcpBuffer::s_ptr buffer) ;
    ~TinyPbCoder(){};
    private:
    const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message,int &len);
};


}


#endif