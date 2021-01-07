#include <asio2/redis/redis.hpp>
#include <iostream>
#include <chrono>
#include <string>
void run_redis_client(std::string_view host, std::string_view port)
{
    auto client = asio2::redis();
    client.auto_reconnect(true, std::chrono::milliseconds(1000)); // enable auto reconnect and use custom delay
    client.start_timer(1, std::chrono::seconds(1), [&client]() {
        client.command({"PING"}, [](const asio::error_code& ec, asio2::redis_reply rep){
            std::cout<<"ping callback:"<<rep.to_string()<<std::endl;
        });
    });
    // client.start_timer(1, std::chrono::milliseconds(1), [&client]() {
    //     static size_t id=0;       
    //      client.command({"GET", "ASIO2"}, [i=id++](const asio::error_code& ec, asio2::redis_reply rep){            
    //          std::cout<<"id:"<<i<<"get callback:"<<rep.to_string()<<std::endl;        
    //          });    
    // }); // test timer
    client.bind_connect([&](asio::error_code ec)
    {
        if (asio2::get_last_error())
            printf("connect failure : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
        else
            printf("connect success : %s %u\n", client.local_address().c_str(), client.local_port());
        client.start_timer(2, std::chrono::milliseconds(1500), [&client]() {
            client.command({"SET", "ASIO2", "HHHHHHHH"},[&client](const asio::error_code& ec, asio2::redis_reply rep) {
                std::cout<<"set callback:"<<rep.to_string()<<std::endl;
                client.command({"GET", "ASIO2"},[&client](const asio::error_code& ec, asio2::redis_reply rep) {
                    std::cout<<"set callback:"<<rep.to_string()<<std::endl;
                });

            });
        });

        client.command({"ZADD", "list", "9", "baidu", "7", "yahoo", "1", "qq", "3", "google"},[&client](const asio::error_code& ec, asio2::redis_reply rep) {
                std::cout<<"set callback:"<<rep.to_string()<<std::endl;
                client.command({"ZRANGE", "list", "0", "-1", "WITHSCORES"},[&client](const asio::error_code& ec, asio2::redis_reply rep) {
                    std::cout<<"set callback:"<<rep.to_string()<<std::endl;
                });

            });

        client.command1([&client](const asio::error_code& ec, asio2::redis_reply rep) {
            std::cout<<"set callback:"<<rep.to_string()<<std::endl;
            client.command({"ZRANGE", "list", "0", "-1", "WITHSCORES"},[&client](const asio::error_code& ec, asio2::redis_reply rep) {
                std::cout<<"set callback:"<<rep.to_string()<<std::endl;
            });
        },
            "ZADD", "list", 2, "facebook", 5, "alibaba"


        );

        //asio::write(client.socket(), asio::buffer(s));
    }).bind_disconnect([&](asio::error_code ec)
    {
        printf("disconnect : %d %s\n", asio2::last_error_val(), asio2::last_error_msg().c_str());
    });
    // client.command("fuck");

    client.start(host, port);

    while (std::getchar() != '\n');

}

int main(int argc, char**){
    const char* host = "127.0.0.1";
	const char* port = "6379";
    run_redis_client(host, port);
    return 0;
}