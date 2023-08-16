#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "rocket/net/rpc/rpc_dispatcher.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/common/log.h"
#include "rocket/common/error_code.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/rpc/rpc_closure.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_connection.h"

namespace rocket{
    #define DELETE_RESOURCE(XX) \
  if (XX != NULL) { \
    delete XX;      \
    XX = NULL;      \
  }                 \

void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request,AbstractProtocol::s_ptr response,TcpConnection* connection)
{
    std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
    std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
    std::string service_name ;
    std::string method_full_name = req_protocol->m_method_name;
    std::string method_name;
    if(parseServiceFullName(method_full_name,service_name,method_name))
    {
        setTinyPBError(rsp_protocol,ERROR_PARSE_SERVICE_NAME,"parse service name error");
        return;
    }
    auto it = m_service_map.find(service_name);
    if(it == m_service_map.end())
    {
        ERRORLOG("%s|service[%s] not found",req_protocol->m_msg_id.c_str(),service_name.c_str());
        setTinyPBError(rsp_protocol,ERROR_SERVICE_NOT_FOUND,"parse service name error");
        return;
    }
    service_s_ptr service = (*it).second;
    const google::protobuf::MethodDescriptor* method=service->GetDescriptor()->FindMethodByName(method_name);
    if(method == NULL)
    {
        ERRORLOG("%s|method name [%s] not found in service[%s]",req_protocol->m_msg_id.c_str(),method_name.c_str(),service_name.c_str());
        setTinyPBError(rsp_protocol,ERROR_METHOD_NOT_FOUND,"parse method name error");
        return;
    }
    google::protobuf::Message* request_msg = service->GetRequestPrototype(method).New();



    if(!request_msg->ParseFromString(req_protocol->m_pb_data))
    {
        ERRORLOG("%s | deserilize error", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
        setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
        DELETE_RESOURCE(request_msg);
        return;     
    }
    INFOLOG("req_id[%s],get rpc request[%s]",req_protocol->m_msg_id.c_str(),request_msg->ShortDebugString().c_str());
    google::protobuf::Message* response_msg = service->GetResponsePrototype(method).New();
    RpcController* rpc_controller = new RpcController();
    rpc_controller->SetLocalAddr(connection->getLocalAddr());
    rpc_controller->SetPeerAddr(connection->getPeerAddr());
    rpc_controller->SetMsgId(req_protocol->m_msg_id);
    service->CallMethod(method,rpc_controller,request_msg,response_msg,NULL);

    if( response_msg->SerializeToString(&rsp_protocol->m_pb_data))
    {
        ERRORLOG("%S | serilize error,origin message [%s]",req_protocol->m_msg_id.c_str(),response_msg->ShortDebugString().c_str());
        setTinyPBError(rsp_protocol,ERROR_FAILED_SERIALIZE,"serilize error");
        DELETE_RESOURCE(response_msg);
        return;
    }
    

    rsp_protocol->m_err_code = 0;

    INFOLOG("%s| dipatch success,request[%s],respone[%s]",req_protocol->m_msg_id.c_str(),request_msg->ShortDebugString().c_str(),response_msg->ShortDebugString().c_str());
   
    DELETE_RESOURCE(response_msg);
    DELETE_RESOURCE(request_msg);

}
void RpcDispatcher::registerService(service_s_ptr service_ptr){
    std::string service_name = service_ptr->GetDescriptor()->full_name();
    m_service_map[service_name] = service_ptr;

}
 void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol>msg,int32_t err_code,const std::string& err_info){
    msg->m_err_code = err_code;
    msg->m_err_info = err_info;
    msg->m_err_info_len = err_info.size();
 }
bool RpcDispatcher::parseServiceFullName(const std::string& full_name,std::string& service_name,std::string& method_name){
    if(full_name.empty())
    {
        ERRORLOG("service full name is empty");
        return false;
    }
    size_t pos = full_name.find_first_of(".");
    if(pos!=std::string::npos)
    {
        service_name = full_name.substr(0,pos);
        method_name = full_name.substr(pos+1);
        INFOLOG("service name[%s],method name[%s]",service_name.c_str(),method_name.c_str());
        return true;
    }else{
        ERRORLOG("invalid service full name[%s]",full_name.c_str());
        return false;

    }
}
}