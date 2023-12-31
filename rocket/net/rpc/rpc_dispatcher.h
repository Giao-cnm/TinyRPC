#ifndef ROCKET_NET_TCP_RPC_RPC_DISPATCHER_H
#define ROCKET_NET_TCP_RPC_RPC_DISPATCHER_H
#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/coder/abstract_protocol.h"
#include<memory>
#include<map>
#include<google/protobuf/service.h>
#include"rocket/common/error_code.h"
#include"rocket/net/coder/tinypb_protocol.h"

namespace rocket{

class TcpConnection;
class RpcDispatcher
{
    public:
    typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;

    void dispatch(AbstractProtocol::s_ptr request,AbstractProtocol::s_ptr response,TcpConnection* connection);

    void registerService(service_s_ptr service_ptr);

    void setTinyPBError(std::shared_ptr<TinyPBProtocol>msg,int32_t err_code,const std::string& err_info);

    private:

    bool parseServiceFullName(const std::string& full_name,std::string& service_name,std::string& method_name);
    
    std::map<std::string,service_s_ptr> m_service_map;


};
}



#endif