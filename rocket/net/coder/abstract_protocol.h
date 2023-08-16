#ifndef ROCKET_NET_CODER_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_CODER_ABSTRACT_PROTOCOL_H
#include<memory>

namespace rocket {
    struct AbstractProtocol:public std::enable_shared_from_this<AbstractProtocol> {
    public:

        virtual ~AbstractProtocol(){};
        typedef std::shared_ptr<AbstractProtocol> s_ptr;
   
      
    private:
    public:
        std::string m_msg_id; //请求id，唯一表示一个请求或者响应
        
    };

    
}
#endif