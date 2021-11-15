#ifndef PUBLIC_H
#define PUBLIC_H

#include <vector>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

class TTimer
{
private:
    struct timeval tv1, tv2;
    struct timezone tz;
    float t_off;
public:
    float value;

    TTimer()
    {
        Clear();
        value=0;
    }

    void Clear(float t=0)
    {
        gettimeofday(&tv1, &tz);
        tv2=tv1;
        t_off=t;
    }

    double GetValue(void)
    {
        double ret;
        gettimeofday(&tv2, &tz);
        ret = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)* 0.000001+t_off;
        value=ret;
        return ret;
    }
};

class Thread
{
private:
    //当前线程的线程ID
    pthread_t tid;
    //线程的状态
    int threadStatus;
    //获取执行方法的指针
    static void * thread_proxy_func(void * args)
    {
        Thread * pThread = static_cast<Thread *>(args);
        pThread->run();
        return NULL;
    }

    //内部执行方法
    void* run1()
    {
        threadStatus = THREAD_STATUS_RUNNING;
        tid = pthread_self();
        run();
        threadStatus = THREAD_STATUS_EXIT;
        tid = 0;
        pthread_exit(NULL);
    }

public:
    //线程的状态－新建
    static const int THREAD_STATUS_NEW = 0;
    //线程的状态－正在运行
    static const int THREAD_STATUS_RUNNING = 1;
    //线程的状态－运行结束
    static const int THREAD_STATUS_EXIT = -1;

    //构造函数
    Thread()
    {
        tid = 0;
        threadStatus = THREAD_STATUS_NEW;
    }

    //线程的运行实体
    virtual void run()=0;

    //开始执行线程
    bool start()
    {
        int iRet = 0;
        pthread_create(&tid, NULL, thread_proxy_func, this) == 0;
    }

    //获取线程ID
    pthread_t getThreadID()
    {
        return tid;
    }

    //获取线程状态
    int getState()
    {
        return threadStatus;
    }

    //等待线程直至退出
    void join()
    {
        if (tid > 0)
        {
            pthread_join(tid, NULL);
        }
    }

    //等待线程退出或者超时
    void join(unsigned long millisTime)
    {
        if (tid == 0)
        {
            return;
        }
        if (millisTime == 0)
        {
            join();
        }else
        {
            unsigned long k = 0;
            while (threadStatus != THREAD_STATUS_EXIT && k <= millisTime)
            {
                usleep(100);
                k++;
            }
        }
    }
};

class TDataFilter
{
private:
    double buf[200];
public:
    int filternum;
    double value;

    TDataFilter(int n)
    {
        filternum=n;
        for(int i=0;i<200;i++) buf[i]=0;
    }

    float GetValue(double v)
    {
        int i;
        for(i=0;i<filternum-1;i++) buf[i]=buf[i+1];
        buf[filternum-1]=v;
        value=0;
        for(i=0;i<filternum;i++) value=value+buf[i];
        value=value/filternum;
        return value;
    }
};

class TAD: public Thread
{
private:
    int fd;
    TDataFilter *df[2];

public:
    float value[2];

    TAD();
    void run();
};


vector<string> split(const string& s, const string& sep);
double P2P(double x1,double y1,double x2,double y2);
bool InPP(double p1,double p2,double px);
vector<string> readFileList(char *basePath);
void ADIn(int ch);
int FindMinID(vector<float> datalist);


#endif //TEST_WSK_PUBLIC_H
