#include"rocket/net/coder/abstract_coder.h"
#include"rocket/net/coder/tinypb_coder.h"
#include"rocket/net/coder/tinypb_protocol.h"
#include<vector>
#include<arpa/inet.h>
#include<string.h>
#include"rocket/common/util.h"
#include"rocket/common/log.h"
namespace rocket{

    void TinyPbCoder::encode(std::vector<AbstractProtocol::s_ptr>&messages,TcpBuffer::s_ptr out_buffer) {
        for(auto &i:messages){
        std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(i);
        int len = 0;
        const char* buf = encodeTinyPB(msg,len);
        if(buf!=NULL&&len!=0)
        {
            out_buffer->writeToBuffer(buf,len);
        }
        if(buf){
     
           free((void*)buf);
           buf = NULL;
        }
        }
        
    }
    void TinyPbCoder::decode(std::vector<AbstractProtocol::s_ptr>&out_messages,TcpBuffer::s_ptr buffer) {
        
        while(1){
        //遍历buffer长度，找到PB_START，找到之后，解析出整包的长度，然后得到结束符的位置，判断是否位PB_END;
        
        std::vector<char> tmp = buffer->m_buffer;
        int start_index = buffer->readIndex();
        int end_index = -1;

        int pk_len = 0;
        bool parse_success = false;
        int i = 0;
        for(int i = start_index;i < buffer->writeIndex();i++){
            if(tmp[i] == TinyPBProtocol::PB_START){
                //y从下取四个字节，由于是网络字节序，需要转为主机字节序
                if(i+1 < buffer->writeIndex())
                {
                    pk_len = getInt32FromByte(&tmp[i+1]);
                    // DEBUGLOG("get pk_len:%d",pk_len);
                    int j = i+pk_len-1;
                    if(j>=buffer->writeIndex()){
                        //说明数据不够，需要等待下一次数据
                        continue;
                    }
                    if(tmp[j] == TinyPBProtocol::PB_END){
                        //说明是一个完整的包
                        start_index = i;
                        end_index = j;
                        parse_success = true;
                        break;
                    }
                }
            }
        }
        if(i>=buffer->writeIndex()){
            //说明没有找到PB_START
            DEBUGLOG("decode end, read all buffer dataT");
            return;
        }
        if(parse_success)
        {
            buffer->moveReadIndex(end_index-start_index+1);
            std::shared_ptr<TinyPBProtocol> msg = std::make_shared<TinyPBProtocol>();
            msg->m_pk_len = pk_len;
            int msg_id_len_index  = start_index+sizeof(char)+sizeof(msg->m_pk_len);
            if(msg_id_len_index >=end_index)
            {
                msg->parse_success = false;
                ERRORLOG("parde error,req_id_len_index[%d],end_index[%d]",msg_id_len_index ,end_index);
                continue;
            }
            msg->m_msg_id_len = getInt32FromByte(&tmp[msg_id_len_index ]);
            DEBUGLOG("get req_id_len:%d",msg->m_msg_id_len );
            int msg_id_index = msg_id_len_index + sizeof(msg->m_msg_id_len);
            char msg_id[100] = {0};
            memcpy(&msg_id[0],&tmp[msg_id_index],msg->m_msg_id_len);
            msg->m_msg_id = std::string(msg_id);

            DEBUGLOG("get msg_id:%s",msg->m_msg_id.c_str());
            int method_name_len_index = msg_id_index + msg->m_msg_id_len;
            if(method_name_len_index>=end_index)
            {
                msg->parse_success = false;
                ERRORLOG("parde error,method_name_len_index[%d],end_index[%d]",method_name_len_index,end_index);
                continue;
            }
            msg->m_method_name_len = getInt32FromByte(&tmp[method_name_len_index]); 
            int method_name_index = method_name_len_index + sizeof(msg->m_method_name_len);
            char method_name[512] ={0};
            memcpy(&method_name[0],&tmp[method_name_index],msg->m_method_name_len);
            msg->m_method_name = std::string(method_name);
            DEBUGLOG("get method_name:%s",msg->m_method_name.c_str());
            int err_code_index = method_name_index + msg->m_method_name_len;
            if(err_code_index>=end_index)
            {
                msg->parse_success = false;
                ERRORLOG("parde error,err_code_index[%d],end_index[%d]",err_code_index,end_index);
                continue;
            }
            msg->m_err_code = getInt32FromByte(&tmp[err_code_index]);
            
            int error_info_len_index = err_code_index + sizeof(msg->m_err_code);
            if(error_info_len_index>=end_index)
            {
                msg->parse_success = false;
                ERRORLOG("parde error,error_info_len_index[%d],end_index[%d]",error_info_len_index,end_index);
                continue;
            }
            msg->m_err_info_len = getInt32FromByte(&tmp[error_info_len_index]);

            int error_info_index = error_info_len_index + sizeof(msg->m_err_info_len);
            char error_info[512] = {0};
            memcpy(&error_info[0],&tmp[error_info_index],msg->m_err_info_len);
            msg->m_err_info = std::string(error_info);
            DEBUGLOG("parse error_info:%s",msg->m_err_info.c_str());

            int pd_data_len = msg->m_pk_len -msg->m_method_name_len - msg->m_msg_id_len - msg->m_err_info_len - 2-24;

            int pd_data_index = error_info_index + msg->m_err_info_len;
            msg->m_pb_data = std::string(&tmp[pd_data_index],pd_data_len);
            //这里校验和解析

            msg->parse_success = true;

            out_messages.push_back(msg);


        }
        }

    }
    const char* TinyPbCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message,int& len){
        if (message->m_msg_id.empty()) {
        message->m_msg_id = "123456789";
        }
        DEBUGLOG("msg_id = %s", message->m_msg_id.c_str());
        int pk_len = 2 + 24 + message->m_msg_id.length() + message->m_method_name.length() + message->m_err_info.length() + message->m_pb_data.length();
        DEBUGLOG("pk_len = %d", pk_len);

        char* buf = reinterpret_cast<char*>(malloc(pk_len));
        char* tmp = buf;

        *tmp = TinyPBProtocol::PB_START;
        tmp++;

        int32_t pk_len_net = htonl(pk_len);
        memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
        DEBUGLOG("pk_len_net = %d", pk_len_net);
        tmp += sizeof(pk_len_net);

        int msg_id_len = message->m_msg_id.length();
        int32_t msg_id_len_net = htonl(msg_id_len);
        memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
        tmp += sizeof(msg_id_len_net);

        if (!message->m_msg_id.empty()) {
        memcpy(tmp, &(message->m_msg_id[0]), msg_id_len);
        tmp += msg_id_len;
        }

        int method_name_len = message->m_method_name.length();
        int32_t method_name_len_net = htonl(method_name_len);
        memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
        tmp += sizeof(method_name_len_net);

        if (!message->m_method_name.empty()) {
        memcpy(tmp, &(message->m_method_name[0]), method_name_len);
        tmp += method_name_len;
        }

        int32_t err_code_net = htonl(message->m_err_code);
        memcpy(tmp, &err_code_net, sizeof(err_code_net));
        tmp += sizeof(err_code_net);

        int err_info_len = message->m_err_info.length();
        int32_t err_info_len_net = htonl(err_info_len);
        memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
        tmp += sizeof(err_info_len_net);

        if (!message->m_err_info.empty()) {
        memcpy(tmp, &(message->m_err_info[0]), err_info_len);
        tmp += err_info_len;
        }

        if (!message->m_pb_data.empty()) {
        memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
        tmp += message->m_pb_data.length();
        }

        int32_t check_sum_net = htonl(1);
        memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
        tmp += sizeof(check_sum_net);

        *tmp = TinyPBProtocol::PB_END;

        message->m_pk_len = pk_len;
        message->m_msg_id_len = msg_id_len;
        message->m_method_name_len = method_name_len;
        message->m_err_info_len = err_info_len;
        message->parse_success = true;
        len = pk_len;

        DEBUGLOG("encode message[%s] success", message->m_msg_id.c_str());

        return buf;
    }
}
      
        
    

