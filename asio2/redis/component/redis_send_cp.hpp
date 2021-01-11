
#ifndef __ASIO_REDIS_REDIS_SEND_OP_HPP__
#define __ASIO_REDIS_REDIS_SEND_OP_HPP__

#include <memory>
#include <future>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

#include <asio/redis/config.hpp>
#include <asio/redis/base/error.hpp>
#include <asio/redis/base/detail/buffer_wrap.hpp>

namespace REDIS::detail
{
	template<class derived_t>
	class redis_send_cp
	{
	protected:
		

	public:
		/**
		 * @constructor
		 */
		redis_send_cp() = default;

		/**
		 * @destructor
		 */
		~redis_send_cp() = default;

	public:
		template<class Callback>
		inline bool command(std::vector<std::string> args, Callback&& callback)
		{
            auto cmd = _build_command(args);
            return this->_derived().send(std::move(cmd), [this, self=this->_derived().selfptr()
                , cb=std::move(callback)](std::size_t bytes_sent){ 
                    // std::cout<<"this:"<<this<<" self:"<<self.get()<<" send callback bytes_sent:"<<bytes_sent<<std::endl;  
                    if(bytes_sent>0)               
                        this->_derived().callback_list_.emplace_back(std::move(cb));                 
                }
            );
			
		}
	private:
    static std::string _build_command(const std::vector<std::string>& args) {
        std::ostringstream oss;
        oss<<"*"<<args.size()<<"\r\n";
        for (const auto& arg : args)
            oss<<"$"<<arg.length()<<"\r\n"<<arg<<"\r\n";
        return oss.str();
        // std::string cmd = "*" + std::to_string(args.size()) + "\r\n";
        // for (const auto& arg : args)
        //     cmd += "$" + std::to_string(arg.length()) + "\r\n" + arg + "\r\n";
        // return cmd;
    }

		inline derived_t & _derived(){return static_cast<derived_t &>(*this);}
	
	};
}

#endif // !__ASIO_REDIS_REDIS_SEND_OP_HPP__
