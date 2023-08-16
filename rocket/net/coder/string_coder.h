#ifndef ROCKET_NET_CODER_STRING_CODER_H
#define ROCKET_NET_CODER_STRING_CODER_H
#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {

class StringProtocol : public AbstractProtocol {
 public:
  std::string info;
};
class StringCoder : public AbstractCoder {
  void encode(std::vector<AbstractProtocol::s_ptr>& messages,
              TcpBuffer::s_ptr out_buffer) {
    for (int i = 0; i < messages.size(); i++) {
      std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);

      out_buffer->writeToBuffer(msg->info.c_str(), msg->info.size());
    }
  }
  void decode(std::vector<AbstractProtocol::s_ptr>& out_messages,
              TcpBuffer::s_ptr buffer) {
    std::vector<char> re;
    buffer->readFromBuffer(re, buffer->readAble());
    std::string info;
    for (int i = 0; i < re.size(); i++) {
      info += re[i];
    }
    std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
    msg->info = info;

    out_messages.push_back(msg);
  }
};

}  // namespace rocket
#endif