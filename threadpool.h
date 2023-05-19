#pragma once
#include<queue>
#include<memory>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<vector>
#include<thread>
#include<iostream>
#include<atomic>
#include<unordered_map>
#include<future>
//线程类
class Thread{
public:
    using threadFunc = std::function<void(int)>;

    Thread(threadFunc func);
    ~Thread();
    //开启线程
    void start();
    int genId(){
        return tid;
    }
private:
    static int genid; //生成线程id 回收线程时候用到
    threadFunc m_func; 
    int tid;//保存线程id
};
//线程池类
class Threadpool{
public:
    Threadpool();
    ~Threadpool();

    template<typename Func,typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        // 获取返回值类型
        using RType = decltype(func(args...));

        //打包任务放入任务队列
        auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func),std::forward<Args>(args)...));
        std::future<RType> reslut = task->get_future();

        //获取锁
        std::unique_lock<std::mutex> locker(m_mutex);
        if(!notFulll.wait_for(locker,std::chrono::seconds(1),
        [&]()->bool{return m_task.size()<m_max_task_size;}))
        {
            //表示等待1s 条件仍然没有满足
            std::cout<<"提交任务失败"<<std::endl;
            auto task  = std::make_shared<std::packaged_task<RType()>>([]()->RType{ return RType();});
            (*task)();
            return task->get_future();
        }

        //如果有空余把任务放到任务队列
        m_task.emplace([task](){(*task)();});
        m_task_size++;
        //notempty 信号通知ß
        notEmpty.notify_all();
        std::cout<<"提交任务成功"<<std::endl;

        return reslut;
}

    void start(unsigned int initsize=4,unsigned int maxsize=8,unsigned int maxtasksize=1024,int maxidletime=10);//开始线程池
    Threadpool(const Threadpool&)=delete;
    Threadpool operator=(const Threadpool&)=delete;
private:
    void threadFunc(int tid);

private:
    //线程列表
    //std::vector<Thread*> m_thread; //线程列表
    //std::vector<std::unique_ptr<Thread>> m_thread; //线程列表
    std::unordered_map<int,std::unique_ptr<Thread>> m_thread;
    unsigned int m_init_thread_size;//初始线程数量
    unsigned int m_max_thread_size;//最大线程数量
    std::atomic_uint16_t m_cur_thread_siz;//当前线程数量
    std::atomic_uint16_t m_idel_thread_size;//空闲线程数


    //任务队列
    using Task = std::function<void()>;
    std::queue<Task> m_task; //任务列表

    unsigned int m_task_size;
    unsigned int m_max_task_size;//最大任务数量
    
    std::mutex m_mutex;
    std::condition_variable notFulll;//任务队列不满的信号量
    std::condition_variable notEmpty;//任务队列不空的信号量
    std::condition_variable exitcond;//离开信号

    //超时回收线程时间
    int m_max_idle_time;

    //结束线程
    std::atomic_bool isPollruning;
};
