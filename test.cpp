#include"threadpool.h"
#include<chrono>
#include<thread>

int sum1(int a){
    //std::cout<<a<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return a;
}
int sum2(int a,int b){
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return a+b;
}
int sum3(int a,int b,int c){
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return a+b+c;
}

int main(){

    {Threadpool pool;
    pool.start(2,4,1024,3);
    
    int (*pr)(int) = sum1;

    std::future<int> r1 = pool.submitTask(*pr,1);
    //std::cout<<"r1:"<<r1.get()<<std::endl;

    std::future<int> r2 = pool.submitTask(sum2,1,2);
    //std::cout<<"r2:"<<r2.get()<<std::endl;

    std::future<int> r3 = pool.submitTask(sum3,1,2,3);
    //std::cout<<"r3:"<<r3.get()<<std::endl;

    std::future<int> r4= pool.submitTask(sum3,1,2,3);
    //std::cout<<"r4:"<<r4.get()<<std::endl;

    std::future<int> r5 = pool.submitTask(sum3,1,2,3);
    //std::cout<<"r5:"<<r5.get()<<std::endl;

    std::future<int> r6 = pool.submitTask(sum3,1,2,3);
    //std::cout<<"r6:"<<r6.get()<<std::endl;
    }

    getchar();
    return 0;
}