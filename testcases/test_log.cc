#include<pthread.h>
#include"rocket/common/log.h"


void* fun(void *)
{
    int i = 20;
    while(i--){
    DEBUGLOG("this thread in %s","22");
    INFOLOG("info this is thread in %s","fun");
    }
    return NULL;

}
int main()
{
    
    rocket::Config::SetGlobalConfig("/home/mispalojar/code/TinyRpc-master/rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    pthread_t th;       
    pthread_create(&th,NULL,&fun,NULL);
    int i =20;
    while(i--){    
    DEBUGLOG("test log %s","11");
    // rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString("test log %s","11") + "\n");
    INFOLOG("test info %s","11");
    }
    //rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + rocket::formatString("test info %s","11") + "\n");
   pthread_join(th,NULL);
    return 0;
}