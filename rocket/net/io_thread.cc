#include"rocket/net/io_thread.h"
#include"rocket/common/log.h"
#include<assert.h>
#include<iostream>

namespace rocket{
    IOThread::IOThread()
    {
        int rt = sem_init(&m_init_semaphore,0,0);
        assert(rt == 0);
        rt = sem_init(&m_start_semaphore,0,0);
        assert(rt == 0);
        pthread_create(&m_thread,NULL,&IOThread::Main,this);
        //wait 知道新线程执行完Main 函数的前置
        sem_wait(&m_init_semaphore);
        DEBUGLOG("IOThread [%d] create success",m_thread_id);

    }
    IOThread::~IOThread()
    {
        m_event_loop->stop();
        sem_destroy(&m_init_semaphore);
         sem_destroy(&m_start_semaphore);
        pthread_join(m_thread,NULL);
        if(m_event_loop)
        {
            m_event_loop->stop();
            delete m_event_loop;
            m_event_loop = NULL;
        }

    }
    EventLoop* IOThread::getEventloop(){
        return m_event_loop;
    }

    void* IOThread::Main(void* arg){
        IOThread* thread = static_cast<IOThread*> (arg);
        thread->m_event_loop = new EventLoop();//异步执行，可能跳过下列代码
        thread->m_thread_id = getThreadId();
        //唤醒等待的线程
        sem_post(&thread->m_init_semaphore);

        DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread %d start loop ", thread->m_thread_id);
        thread->m_event_loop->loop();

        DEBUGLOG("IOThread %d end loop",thread->m_thread_id);
        return NULL;
    }

    void IOThread::start()
    {
        DEBUGLOG("Now invoke IOThread %d",m_thread_id);
        
        sem_post(&m_start_semaphore);
       

    }

    void IOThread::join()
    {
        pthread_join(m_thread,NULL);
    }

}