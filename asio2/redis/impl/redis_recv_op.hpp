
#ifndef __ASIO2_REDIS_RECV_OP_HPP__
#define __ASIO2_REDIS_RECV_OP_HPP__


#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/redis/impl/error.hpp>
#include <asio2/redis/impl/redis_reply_parser.hpp>

namespace asio2::detail
{
	template<class derived_t>
	class redis_recv_op
	{
	public:
		/**
		 * @constructor
		 */
		redis_recv_op() = default;

		/**
		 * @destructor
		 */
		~redis_recv_op() = default;

	protected:
		void _redis_post_recv(std::shared_ptr<derived_t> this_ptr)
		{
			// std::cout<<__FUNCTION__<<std::endl;
			if (!_derived().is_started())
				return;

			try
			{

                asio::async_read(_derived().stream(), _derived().buffer().base(), asio::transfer_at_least(1),/*condition(),*/
                    make_allocator(_derived().rallocator(),
                        [this, self_ptr = std::move(this_ptr)](const error_code & ec, std::size_t bytes_recvd)
                {
                    _derived()._handle_recv(ec, bytes_recvd, std::move(self_ptr));
                }));

			}
			catch (system_error & e)
			{
				set_last_error(e);
				_derived()._do_disconnect(e.code());
			}
		}

		void _redis_handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr)
		{
			// std::cout<<"recv:"<<bytes_recvd<<std::endl;
			set_last_error(ec);

			// bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.
			if (!ec)
			{
				// every times recv data,we update the last active time.
				_derived().update_alive_time();

				size_t pos = 0;
                for(; pos < bytes_recvd;)
                {
                    std::pair<size_t, redis_reply_parser::ParseResult> result =
                        parser_.parse((const char*)_derived().buffer().data().data() + pos, bytes_recvd - pos);
                    pos += result.first;
                    // ::memmove(buf_.data(), buf_.data() + pos, buf_used_ - pos);
                    // buf_used_ -= pos;
                    if( result.second == redis_reply_parser::Completed )
                    {
						if(parser_.result().is_message())
						{
							this->_derived()._fire_recv(this_ptr, std::move(parser_.result()));
						}
						else
						{	
							if(!this->_derived().callback_list_.empty()){
								std::cout<<"before callback:"<<parser_.result().to_string()<<std::endl;
								this->_derived().callback_list_.front()(error_code(), std::move(parser_.result()));
								this->_derived().callback_list_.pop_front();
							}
							else
							{
								ASIO2_ASSERT(false);
							}
						}
                        
                        // std::cout<<"buf:\n"<<std::string(buf_.data(), buf_used_)<<std::endl;
                        continue;
                    }
                    else if( result.second == redis_reply_parser::Incompleted )
                    {
                        continue;
                    }
                    else
                    {
						this->_derived()._do_disconnect(make_error_code(redis_errors::parse_failure));
						// if(!this->_derived().callback_list_.empty()){
						// 	this->_derived().callback_list_.front()(make_error_code(redis_errors::parse_failure), redis_reply());
						// 	this->_derived().callback_list_.pop_front();
						// }
						// else
						// {
						// 	ASIO_REDIS_ASSERT(false);
						// }
                        return ;
                    }
                }

				_derived().buffer().consume(bytes_recvd);
				static condition_wrap<void> condition;
				_derived()._post_recv(std::move(this_ptr), condition);
			}
			else
			{
				_derived()._do_disconnect(ec);
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}
    protected:
        redis_reply_parser parser_;
		protected:
		std::list<std::function<void(const error_code& , redis_reply)>> callback_list_;
	private:
		inline derived_t & _derived(){return static_cast<derived_t &>(*this);}
	};
}

#endif // !__ASIO2_REDIS_RECV_OP_HPP__
