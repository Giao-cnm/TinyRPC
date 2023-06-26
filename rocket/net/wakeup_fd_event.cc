#include"rocket/net/wakeup_fd_event.h"
#include"rocket/common/log.h"
#include<unistd.h>
namespace rocket
{
    WakeUpEvent::WakeUpEvent(int fd):FdEvent(fd)
    {
        init();
    }
    WakeUpEvent::~WakeUpEvent()
    {

    }
  
    void WakeUpEvent::wakeup()
    {
        char buf[8] = {'a'};
        int rt = write(m_fd,buf,8);
        if(rt!=8)
        {
            ERRORLOG("write to wakeup fd less than 8 bytes,fd[%d]",m_fd);
        }
    }
}