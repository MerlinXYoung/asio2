
#ifndef __ASIO2_REDIS_SEND_OP_HPP__
#define __ASIO2_REDIS_SEND_OP_HPP__

#include <memory>
#include <future>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>
#include <asio2/redis/impl/redis_reply.hpp>
#include <asio2/base/detail/util.hpp>
namespace asio2::detail
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

		template<class Callback, class... Args>
		inline bool command1(Callback&& callback, Args&&... args)
		{
            auto cmd = _build_command(args...);
            return this->_derived().send(std::move(cmd), [this, self=this->_derived().selfptr()
                , cb=std::move(callback)](std::size_t bytes_sent){ 
                    // std::cout<<"this:"<<this<<" self:"<<self.get()<<" send callback bytes_sent:"<<bytes_sent<<std::endl;  
                    if(bytes_sent>0)               
                        this->_derived().callback_list_.emplace_back(std::move(cb));                 
                }
            );
			
		}

		inline std::future<redis_reply> command(std::vector<std::string> args)
		{
            auto cmd = _build_command(args);
			std::promise<redis_reply> p;
    		std::future<redis_reply> f = p.get_future();
			auto callback= [p = std::move(p)](asio::error_code ec, redis_reply reply)mutable{
				if(ec){
					p.set_exception(make_exception_ptr(asio::system_error (ec, "redis command")));
					return;

				}
				p.set_value(std::move(reply));
			};
            this->_derived().send(std::move(cmd), [this, self=this->_derived().selfptr()
                , cb=std::move(callback)](std::size_t bytes_sent){ 
                    // std::cout<<"this:"<<this<<" self:"<<self.get()<<" send callback bytes_sent:"<<bytes_sent<<std::endl;  
                    if(bytes_sent>0)               
                        this->_derived().callback_list_.emplace_back(std::move(cb));                 
                }
            );
			return f;
			
		}
	private:

    static std::string _build_command(const std::vector<std::string>& args) {
        std::ostringstream oss;
        oss<<"*"<<args.size()<<"\r\n";
        for (const auto& arg : args)
            oss<<"$"<<arg.length()<<"\r\n"<<arg<<"\r\n";
        return oss.str();

    }
	
	static void _build_cmd_item(std::ostringstream& oss, const std::string& v){
		oss<<"$"<<v.length()<<"\r\n"<<v<<"\r\n";
	}
	static void _build_cmd_item(std::ostringstream& oss, const char* v){
		oss<<"$"<<strlen(v)<<"\r\n"<<v<<"\r\n";
	}
	static void _build_cmd_item(std::ostringstream& oss, uint32_t v){
		_build_cmd_item(oss, std::to_string(v));
	}
	static void _build_cmd_item(std::ostringstream& oss, int32_t v){
		_build_cmd_item(oss, std::to_string(v));
	}
	static void _build_cmd_item(std::ostringstream& oss, uint64_t v){
		_build_cmd_item(oss, std::to_string(v));
	}
	static void _build_cmd_item(std::ostringstream& oss, int64_t v){
		_build_cmd_item(oss, std::to_string(v));
	}
	

	template<class... Args>
	static std::string _build_command(const Args&... args) {
        std::ostringstream oss;
        oss<<"*"<<sizeof...(Args)<<"\r\n";
		_build_command(oss, args...);
        return oss.str();
  
    }

	template<class T, class... Args>
	static void _build_command(std::ostringstream& oss, T& v,const Args&... args) {
		_build_cmd_item(oss,v);
        _build_command(oss, args...);
  
    }

	template<class T, class... Args>
	static void _build_command(std::ostringstream& oss, T& v) {
		_build_cmd_item(oss,v);
    }

		inline derived_t & _derived(){return static_cast<derived_t &>(*this);}
	
	};
}

#endif // !__ASIO2_REDIS_SEND_OP_HPP__
