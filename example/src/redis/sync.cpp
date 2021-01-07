#include <asio2/redis/redis.hpp>
#include <string>
#include <iostream>
int main(int argc, char** argv)
{
    // ASIO::io_context io_ctx;
    // ASIO::error_code ec;
    // ASIO_REDIS::tcp_client cli(io_ctx);
    // auto addr = ASIO::ip::address::from_string("127.0.0.1");
    // ASIO::ip::tcp::endpoint ep(addr, 6379);
    // ASIO_REDIS::reply r;
    // cli.connect(ep);
    // cli.send<std::string>({"SET", "A", "VALUE_A"}, ec);
    // cli.read(r, ec);
    // if(ec)
    // {
    //     std::cerr<<"fuck error:"<<ec.message()<<std::endl;
    //     return -1;
    // }
    // std::cout<<r.to_string()<<std::endl;

    // cli.send<std::string>({"GET", "A"}, ec);
    // cli.read(r, ec);
    // if(ec)
    // {
    //     std::cerr<<"fuck error:"<<ec.message()<<std::endl;
    //     return -1;
    // }
    // std::cout<<r.to_string()<<std::endl;

    return 0;
}