#include <iostream>
#include <vector>
#include <stdlib.h>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <math.h>
#include <stdlib.h>
#include "fcntl.h"
#include <sys/wait.h>
#include <memory.h>
#include <condition_variable>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <ctime>
#include <iomanip>

std::mutex mutex;
std::condition_variable cv;
constexpr int buf_size = 10;
int buf[buf_size];
bool cond1 = false;

void worker_thread()
{
    while(true)
    {
        
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout  << "thread id: " << std::this_thread::get_id() << "," << std::put_time(std::localtime(&time),"%F %T") << std::endl;;
        
        {
            std::unique_lock uq(mutex);
            //while(!cond1)
            //{
                cv.wait(uq,[](){return cond1;});
            //}
            cond1 = false; 
            //uq.unlock();
        }
        cv.notify_one();
    }
}

void producer_thread()
{
    while(true)
    {
        {
            std::unique_lock lg(mutex);
            cond1 = true;
            //lg.unlock();
        }
        cv.notify_one();
        {
            std::unique_lock uq (mutex);
            //while(cond1)
            //{
                cv.wait(uq,[](){return !cond1;});
            //}
        }
        //uq.unlock();
    }
}



int main(int argc,char* argv[])
{
    memset(buf,0,buf_size * sizeof(int));


    std::thread t1(worker_thread);
    t1.detach();
    std::thread t2(producer_thread);
    t2.detach();

    std::getchar();

    return 0;
}
