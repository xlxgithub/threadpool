#include "threadpool.h"
Threadpool::Threadpool():m_task_size(0)
{

}

Threadpool::~Threadpool()
{
    isPollruning = false;//停止运行
    std::cout<<"start退出线程池"<<std::endl;

    std::unique_lock<std::mutex> locker(m_mutex);
    notEmpty.notify_all(); //唤醒所有消费者线程

    exitcond.wait(locker,[&]()->bool{return m_cur_thread_siz==0;});
    std::cout<<"退出线程池"<<std::endl;
}

Reslut Threadpool::submitTask(std::shared_ptr<Task> sp)
{
    //获取锁

    std::unique_lock<std::mutex> locker(m_mutex);
    //线程通信 等到任务队列有空余
    // while (m_thread.size()==m_max_task_size)
    // {
    //     notFulll.wait(locker);
    // }
    //wait wait_for wait_until
    //notFulll.wait(locker,[&]()->bool{return m_thread.size()<m_max_task_size;});
    if(!notFulll.wait_for(locker,std::chrono::seconds(1),
    [&]()->bool{return m_task.size()<m_max_task_size;}))
    {
        //表示等待1s 条件仍然没有满足
        std::cout<<"提交任务失败"<<std::endl;
        return Reslut(sp,false);
    }
    //std::cout<<"tid:"<<std::this_thread::get_id()<<"获取任务成功"<<std::endl;
    //如果有空余把任务放到任务队列上
    m_task.emplace(sp);
    m_task_size++;
    //notempty 信号通知
    notEmpty.notify_all();


    if(m_idel_thread_size<m_task.size() && m_cur_thread_siz<m_max_thread_size && m_task.size()>0){
        for (size_t i = m_idel_thread_size; i < m_task.size(); i++)
        {
                auto pr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc,this,std::placeholders::_1));
                //m_thread.emplace_back(std::move(pr));
                int threadid = pr->genId();
                m_thread.emplace(threadid,std::move(pr));
                m_thread[i]->start();
                //m_thread.push_back(new Thread(std::bind(&Threadpool::threadFunc,this)));
                m_idel_thread_size += 1;
                m_cur_thread_siz += 1;
                std::cout<<"新建线程0 id:"<<threadid<<std::endl;
                std::cout<<"task szie:"<<m_task.size()<<std::endl;
        } 
  }
    return Reslut(sp);

}

void Threadpool::start(unsigned int initsize,unsigned int maxsize,unsigned int maxtasksize,int maxidletime)
{
    m_init_thread_size = initsize;
    m_max_thread_size = maxsize;
    m_max_task_size = maxtasksize;
    m_max_idle_time  = maxidletime;
    isPollruning = true;
    for (size_t i = 0; i < m_init_thread_size; i++)
    {
        auto pr = std::make_unique<Thread>(std::bind(&Threadpool::threadFunc,this,std::placeholders::_1));
        int threadid = pr->genId();
        m_thread.emplace(threadid,std::move(pr));
        //m_thread.emplace_back(std::move(pr));
        //m_thread.push_back(new Thread(std::bind(&Threadpool::threadFunc,this)));
    }
    m_cur_thread_siz = m_init_thread_size;
    for (size_t i = 0; i < m_init_thread_size; i++)
    {
        m_thread[i]->start();
    }
    m_idel_thread_size = m_init_thread_size;   
    std::cout<<"当前线程池线程数量:"<<m_thread.size()<<std::endl;
}

void Threadpool::threadFunc(int tid)
{
    std::cout<<"线程开始 ID:"<<tid<<std::endl;
    std::cout<<"当前线程池空闲线程数量:"<<m_idel_thread_size<<std::endl;
    // m_curtime = std::chrono::time_point();
    //auto lasttime = std::chrono::high_resolution_clock().now();
    std::chrono::time_point lasttime = std::chrono::high_resolution_clock().now();
    for(;;)
    {
        
        if(!isPollruning && m_task.size()==0){
            m_cur_thread_siz=0;
            exitcond.notify_all();
            return;
        }
        std::shared_ptr<Task> sp = nullptr;
       {
            //获取锁
            std::unique_lock<std::mutex>locker(m_mutex);
            //cached模式下创建新线程结束后 超时10s回收线程
            if(m_cur_thread_siz>m_init_thread_size){
                while (m_task.size()==0 && m_cur_thread_siz > m_init_thread_size)
                {
                    if(std::cv_status::timeout==notEmpty.wait_for(locker,std::chrono::seconds(1))){
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now-lasttime);
                        if(dur.count()>= m_max_idle_time  && m_cur_thread_siz > m_init_thread_size){
                            //回收当前线程
                            m_thread.erase(tid);
                            m_cur_thread_siz--;
                            m_idel_thread_size--;
                            std::cout<<tid<<"：线程退出"<<std::endl;
                            std::cout<<"当前线程数量size:  "<<m_thread.size()<<std::endl;
                            std::cout<<"当前线程数量: "<<m_cur_thread_siz<<std::endl;
                            std::cout<<"当前空闲线程数量: "<<m_idel_thread_size<<std::endl;
                        }
                    }
                }
                
            }
    
            if(m_task.size()>0){
                notEmpty.wait(locker,[&]()->bool{return m_task.size()>0;});
    
                //从任务队列中取出一个任务
                sp = m_task.front();
                m_task.pop();
                m_task_size--;
                
                //如果还有任务通知其他线程
                if(m_task_size>0){
                    notEmpty.notify_all(); 
                }
                notFulll.notify_all();
            }
            //释放锁
       }

            //当前线程执行任务
            if(sp!=nullptr){
                std::cout<<"当前线程tid: "<<tid<<"获取任务成功"<<std::endl;
                m_idel_thread_size -= 1;
                //std::cout<<"执行前空闲线程数量"<<m_idel_thread_size<<std::endl;
                sp->exec(); 
                m_idel_thread_size += 1;
                //std::cout<<"执行后空闲线程数量"<<m_idel_thread_size<<std::endl;
            }
            lasttime = std::chrono::high_resolution_clock().now();
        
    }
    if(!isPollruning){
        m_cur_thread_siz=0;
        exitcond.notify_all();
        return;
    }
}

/*******************************线程方法实现********************************************/
int Thread::genid=0;
Thread::Thread(threadFunc func):m_func(func),tid(genid++)
{
}

Thread::~Thread()
{
}

void Thread::start()
{
    //创建一个线程来执行线程函数
    std::thread t(m_func,tid);
    t.detach();
}

/*******************************返回值方法实现********************************************/
Reslut::Reslut(std::shared_ptr<Task> task, bool isvalid):m_task(task),m_isvalid(isvalid)
{
    task->setReslut(this);
}

Any Reslut::get()
{
   if(!m_isvalid){
    return " ";
   }
   m_sem.wait(); //task 任务没有执行完 这里会阻塞
   return std::move(m_any);
}

void Reslut::setVal(Any any)
{
    //存储task返回值
    this->m_any = std::move(any);
    m_sem.post();
}

/*******************************任务类方法实现********************************************/
void Task::exec()
{
   if(m_reslut!=nullptr){
     m_reslut->setVal(run());
   }
}
void Task::setReslut(Reslut* res)
{
    m_reslut = res;
}
Task::Task() : m_reslut(nullptr)
{
    
}
Task::~Task()
{
}
