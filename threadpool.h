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

//Any类 存储任意类型元素 C++17中有
class Any{
public:
    Any()=default;
    ~Any()=default;
    Any(Any&&)=default;
    Any& operator=(Any&&) = default;
    //构造函数 让any接受任意类型数据
    template<class T>
    Any(T data):m_base(std::make_unique<Derive<T>>(data)){};
    // Any(T data){
    //     m_base(new Derive<T>(data));
    // }
    //存储出来的数据提取出来
    template<class T>
    T cast_(){
        //基类指针 到派生类指针
        Derive<T>* dptr = dynamic_cast<Derive<T>*>(m_base.get());
        if(dptr==nullptr){
            throw "类型错误";
        }
        return dptr->m_data;
    }
private:
    //基类类型
    class Basic{
    public:
        virtual ~Basic()=default;
    };
    //派生类类型
    template<class T>
    class Derive :public Basic{
    public:
        Derive(T data):m_data(data){};
        T m_data; //保存任意的其他类型
    };
private:
    std::unique_ptr<Basic> m_base; //定义一个基类指针
};
//信号量类 linux sem_init c++20 semaphore
class Semaphore{
public:
    Semaphore():m_count(0){};
    Semaphore(int count):m_count(count){};
    ~Semaphore()=default;

    //获取一个信号量
    void wait(){
        std::unique_lock<std::mutex> locker(m_mutex);
        m_cond.wait(locker,[&]()->bool{return m_count>0;});
        m_count--;
    }
    //释放一个信号量
    void post(){
        std::unique_lock<std::mutex> locker(m_mutex);
        m_count++;
        m_cond.notify_all();
    }

private:
    int m_count;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};
//实现接受任务返回值
class Task;
class Reslut{
public:
    Reslut()=default;
    Reslut(std::shared_ptr<Task> task,bool isvalid=true);
    Any get();
    void setVal(Any any);
    ~Reslut()=default;
private:
    Any m_any;
    Semaphore m_sem;
    std::shared_ptr<Task> m_task;
    std::atomic_bool m_isvalid;
};

//任务类
class Task{
public:
    Task();
    ~Task();
    void exec();
    void setReslut(Reslut* res);
    //用户可以自定义任务类型 从Task继承 重写run函数 
    virtual Any run()=0;
private:
    Reslut* m_reslut;
};
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
    Reslut submitTask(std::shared_ptr<Task> sp);//给线程池提交任务
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
    std::queue<std::shared_ptr<Task>> m_task; //任务列表
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
