#include"rocket/net/fd_event.h"
#include"rocket/common/log.h"
#include<iostream>
#include<string.h>
namespace rocket{
    FdEvent::FdEvent()
    {
        memset(&m_listen_events,0,sizeof(m_listen_events));
    }
    FdEvent::FdEvent(int fd):m_fd(fd)
    {
        memset(&m_listen_events,0,sizeof(m_listen_events));
    }

    FdEvent::~FdEvent()
    {

    }

    /*
    这段代码实现了根据传入的事件类型，返回对应的事件回调函数。当需要处理输入事件时，返回之前保存的输入事件回调函数
    */
    std::function<void()> FdEvent::handler(TriggerEvent event)
    {
        if(event == TriggerEvent::IN_EVENT)
        {
           return m_read_callback;
        }else{
           return m_write_callback;
        }
    }
    /*
    这段代码实现了根据传入的事件类型设置相应的事件标志，并保存对应的回调函数，以便在事件发生时执行回调函数
    */
    void FdEvent::listen(TriggerEvent event_type,std::function<void()>callback)
    {

        if(event_type == TriggerEvent::IN_EVENT)
        {

            m_listen_events.events |= EPOLLIN;
            
            m_read_callback = callback;
            
            
        }else{
            m_listen_events.events |= EPOLLOUT;
            m_read_callback = callback;
        }

        m_listen_events.data.ptr = this;


    }

}