#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/eventfd.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/util.h"
#include "rocket/common/log.h"
#include"rocket/common/config.h"
#include"rocket/common/mutex.h"
#include<string.h>
#include<iostream>

#define ADD_TO_EPOLL()                                                                      \
auto it = m_listen_fds.find(event->getFd());                                                \
int op = EPOLL_CTL_ADD;                                                                     \
if(it != m_listen_fds.end())                                                                \
{                                                                                           \
    op = EPOLL_CTL_MOD;                                                                     \
}                                                                                           \
epoll_event tmp = event->getEpollEvent();                                                   \
INFOLOG("epoll_event.events = %d",(int)tmp.events)                                          \
int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp);                                      \
if(rt == -1)                                                                                \
{                                                                                           \
    ERRORLOG("fai\led epoll_ctl when add fd , errno = %d,error =%s",errno,strerror(errno)); \
}                                                                                           \
DEBUGLOG("add event success, fd[%d]",event->getFd())\

#define DELETE_TO_EPOLL()                                                                   \
 auto it = m_listen_fds.find(event->getFd());                                               \
int op = EPOLL_CTL_DEL;                                                                     \
if(it == m_listen_fds.end())                                                                \ 
{                                                                                           \
    return;                                                                                 \
}                                                                                           \
epoll_event tmp = event->getEpollEvent();                                                   \
int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp);                                      \
if(rt == -1)                                                                                \
{                                                                                           \
    ERRORLOG("failed epoll_ctl when add fd, errno=%d,error=%s",errno,strerror(errno));      \
}                                                                                           \
DEBUGLOG("delete event success,fd[%d]",event->getFd());                                     \

namespace rocket{
    static thread_local EventLoop* t_current_eventloop = NULL;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    EventLoop::EventLoop() // 创建epoll实例，并且注册监听事件
    {
        
        if(t_current_eventloop!=NULL)
        {
            ERRORLOG("failed to create event loop ,this thread has created event loop");
            exit(0);
        }

        m_thread_id = getThreadId();
        m_epoll_fd = epoll_create(10);
        if(m_epoll_fd == -1 ) 
        {
            ERRORLOG("failed to create event loop ,epoll_create error,error info[%d]",errno);
            exit(0);
        }

        initWakeUpFdEvent();
        initTimer();

        INFOLOG("succ create event loop in thread %d",m_thread_id);
        t_current_eventloop = this;

        
    }

    EventLoop::~EventLoop(){
        close(m_epoll_fd);
        if(m_wakeup_fd_event)
        {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = NULL;
        }
        if(m_timer)
        {
            delete m_timer;
            m_timer = NULL;
        }
    }
    void EventLoop::initWakeUpFdEvent()  //初始化 事件文件符以及对应的回调函数
    {
        m_wakeup_fd = eventfd(0,EFD_NONBLOCK); //创建一个事件文件描述符
        if (m_wakeup_fd<0)
        {
            ERRORLOG("failed to create event loop, epoll_create error info[%d]",errno);
            exit(0);
        }
        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

        m_wakeup_fd_event->listen(FdEvent::IN_EVENT,[this](){
            char buf[8];
            std::cout<<"sss"<<std::endl;  
            while(read(m_wakeup_fd,buf,8)!=-1&& errno != EAGAIN) //读事件的回调函数实现
            {
                
            }
            DEBUGLOG("read full bytes fromm wakeup fd[%d]",m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
    }

    void EventLoop::loop()
    {
        while(!m_stop_flag)
        {
            ScopeMutex<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();
            while(!tmp_tasks.empty())
            {
               std::function<void()> cb =  tmp_tasks.front();
                tmp_tasks.pop();
                if(cb)
                {
                    cb();
                }
            }
            //如果有定时任务需要执行，那么执行
            // 1.怎么判断一个定时任务需要执行？ （now）>TimerEvent.arrive_time
            //2.  怎么监听？准确返回
            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];
            // DEBUGLOG("now begin to epoll wait");
            int rt = epoll_wait(m_epoll_fd,result_events,g_epoll_max_events,timeout);//监听所有fd事件
            DEBUGLOG("now end epoll wait, rt = %d",rt);
            if(rt<0) {
                ERRORLOG("epoll_wait error,errno=",errno);
            }else{
                for(int i = 0;i<rt;++i)//返回套接字的个数 
                {
                    epoll_event trrigger_event = result_events[i];
                    FdEvent* fd_event = static_cast<FdEvent* > (trrigger_event.data.ptr);
                    if(fd_event == NULL) continue;
                    if(trrigger_event.events & EPOLLIN )
                    {
                        DEBUGLOG("fd %d trigger in event",fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::IN_EVENT));
                    }
                     if(trrigger_event.events & EPOLLOUT )
                    {
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }
                }
            }
        }
    }

    void EventLoop::wakeup()
    {
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }
    void EventLoop::dealWakeup()
    {

    }

    void EventLoop::stop()
    {
        m_stop_flag = true;
    }

    void EventLoop::addEpollEvent(FdEvent* event){ 
        // TODO
        if(isInLoopThread())
        {
            ADD_TO_EPOLL();
        }else{
            auto cb = [this,event]()
            {
                ADD_TO_EPOLL();
            };
            addTask(cb,true);
        }

    }
    void EventLoop::initTimer()
    {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }
    void EventLoop::addTimerEvent(TimerEvent::s_ptr event)
    {
        m_timer->addTimerEvent(event);
    }
    void EventLoop::addTask(std::function<void()> cb,bool is_wake_up)
    {
        ScopeMutex<Mutex> lock(m_mutex);
        m_pending_tasks.push(cb);
        lock.unlock();
        if(is_wake_up)
        {
            wakeup();
        }
    }
    void EventLoop::deleteEpollEvent(FdEvent* event)
    {
        if(isInLoopThread())
        {
            auto it = m_listen_fds.find(event->getFd()); 
            int op = EPOLL_CTL_DEL; 
            if(it == m_listen_fds.end()) 
            { 
               return;
            } 
            epoll_event tmp = event->getEpollEvent(); 
            int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp); 
            if(rt == -1) { 
                ERRORLOG("failed epoll_ctl when add fd, errno=%d,error=%s",errno,strerror(errno));
            }
            DEBUGLOG("delete event success,fd[%d]",event->getFd());
        }else{
            auto cb = [this,event]()
            {
            auto it = m_listen_fds.find(event->getFd()); 
            int op = EPOLL_CTL_DEL; 
            if(it == m_listen_fds.end()) 
            { 
               return;
            } 
            epoll_event tmp = event->getEpollEvent(); 
            int rt = epoll_ctl(m_epoll_fd,op,event->getFd(),&tmp); 
            if(rt == -1) { 
                ERRORLOG("failed epoll_ctl when add fd, errno=%d,error=%s",errno,strerror(errno));
            }
            DEBUGLOG("delete event success,fd[%d]",event->getFd());
            };
        }
    }
    bool EventLoop::isInLoopThread()
    {
        return getThreadId() == m_thread_id;
    }
 
}