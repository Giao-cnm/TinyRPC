## 1.1日志模块开发
首先需要创建项目

日志模块开发
```
1. 日志级别
2. 打印到文件，支持日期命名，以及日志的滚动
3. C格式风格
4. 线程安全问题
```
LogLevel:
```
DeBug
Info
Error
```
LogEvent
```
文件名、行号
MsgNo
Thread id 
日期、以及时间，精确到ms
自定义消息
```
日志格式
```
[%y-%m-%d %H:%M:%s.%ms]\t[pid::thread_id]\t [file_name:line][%msg]
```

Logger 日志器
```
1. 提供打印日志方法
2. 设置日志输出路径
```

## 2.Reactor

Reactor,又可以称为EventLoop，它本质是一个事件循环模型，核心是一个loop循环
```
void loop()
{
    while(!stop)
    {
        foreach(task in tasks)
        {
            task();
        }
        // 1.取得下次定时任务的事件，与设定的timer_out去比较大小，若下次定时任务时间超过1s就取下次定时任务为超时时间，否则取1s
        int time_out = Max(1000,getNextTimerCallback());
        int rt = epol_wait(epfd,fds,...,time_out);
        if(rt<0)
        {
            foreach(fd in fds)
            {
                tasks.push();
            }
        }
    }
}
```

### 2.1 TimerEvent 定时任务
```
1.指定时间点 arrive_time
2.interval,ms //间隔
3.is_repeated //周期性
4.is_cancled 
5.task

cancle()
cancleRepeated()
```
### 2.2 Timer
定时器，他是一个TimerEvent集合。Timer继承FdEvent
```
addTimerEvent();
deleteTimerEvent();
onTimer();
resetArriveTime();
multimap 存储TimerEvent<key(arrivetime),TimerEvent>
```