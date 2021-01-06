#include <asio2/base/detail/util.hpp>
#include <iostream>
#include <netinet/in.h>
#include <sys/time.h>

class Timer{
    timeval tv;
public:
    Timer(){
        gettimeofday(&tv, 0);
    }
    ~Timer(){
        timeval tve;
        gettimeofday(&tve, 0);
        std::cout<<"Timer:"<<tve.tv_sec-tv.tv_sec<<"."<<tve.tv_usec-tv.tv_usec<<std::endl;
    }
};

template<class T>
void swap(T& a, T& b){
    a^=b;
    b^=a;
    a^=b;
}


int main(int argc, char** args){
    uint32_t s=1;
    std::cout<<htonl(s) <<std::endl;
    std::cout<<asio2::detail::hton(s)<<std::endl;
    size_t a=10,b=9999999;
    {
        Timer timer;
        for(size_t i=0;i<100000; ++i){
            std::swap(a,b);
        }
    }

    {
        Timer timer;
        for(size_t i=0;i<100000; ++i){
            swap(a,b);
        }
    }

    uint32_t aa=10,bb=9999999;
    {
        Timer timer;
        for(size_t i=0;i<100000; ++i){
            std::swap(aa,bb);
        }
    }

    {
        Timer timer;
        for(size_t i=0;i<100000; ++i){
            swap(aa,bb);
        }
    }

    uint32_t ab=999,ac=666;
    swap(ab,ac);
    std::cout<<"ab:"<<ab<<"\tac:"<<ac<<std::endl;

    return 0;
}