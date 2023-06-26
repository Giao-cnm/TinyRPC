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