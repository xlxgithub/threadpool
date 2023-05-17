#include"threadpool.h"
#include<chrono>
#include<thread>
class mytask:public Task{
public:
    mytask(int a,int b):begin(a),end(b){};
    Any run(){
        std::cout<<"tid: "<<std::this_thread::get_id()<<" begin"<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout<<"tid: "<<std::this_thread::get_id()<<" end"<<std::endl;
        int sum = 0;
        // for (int i = begin; i < end; i++)
        // {
        //     /* code */
        //     sum += i;
        // }
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        return sum;

    }
    int begin;
    int end;
};
int main(){

    /*
        
    */
       {
        Threadpool pool;
        pool.start(4,8,1024,3);
        Reslut res0 = pool.submitTask(std::make_shared<mytask>(1,10));
        // int sum0 = res0.get().cast_<int>();
        // std::cout<<"Sum0: "<<sum0<<std::endl; 
        }
 

    // Reslut res1 = pool.submitTask(std::make_shared<mytask>(2,10));


    // Reslut res2 = pool.submitTask(std::make_shared<mytask>(3,10));


    // Reslut res3 = pool.submitTask(std::make_shared<mytask>(5,10));


    // Reslut res4 = pool.submitTask(std::make_shared<mytask>(7,10));


    // Reslut res5 = pool.submitTask(std::make_shared<mytask>(1,10));


    // Reslut res6 = pool.submitTask(std::make_shared<mytask>(1,10));
 

    // Reslut res7 = pool.submitTask(std::make_shared<mytask>(1,10));


    // Reslut res8= pool.submitTask(std::make_shared<mytask>(1,10));





    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    // pool.submitTask(std::make_shared<mytask>());
    Any a(0);
    //std::cout<<a.cast_()<<std::endl;
    std::cout<<"Sum0"<<std::endl; 
    getchar();
}