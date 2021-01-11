
#ifndef __ASIO2_REDIS_CLIENT_HPP__
#define __ASIO2_REDIS_CLIENT_HPP__

#include <asio2/tcp/tcp_client.hpp>


#include <asio2/redis/component/redis_send_cp.hpp>
#include <asio2/redis/impl/redis_recv_op.hpp>

namespace asio2::detail
{
	struct template_args_redis:public template_args_tcp_client
	{

	};
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT1;
	template<class derived_t, class args_t>
	class redis_impl_t
		: public tcp_client_impl_t<derived_t, args_t>
		, public redis_send_cp<derived_t>
		, public redis_recv_op<derived_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT1;
		template <class>         friend class redis_send_cp;
		template <class>         friend class redis_recv_op;

	public:
		using self = redis_impl_t<derived_t, args_t>;
		using super = tcp_client_impl_t<derived_t, args_t>;
		using buffer_type = typename super::buffer_type;

		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit redis_impl_t(
			std::size_t init_buffer_size = tcp_frame_size, 
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			Args&&... args
		)
			: super(init_buffer_size, max_buffer_size, args...)
			, redis_send_cp<derived_t>()
			, redis_recv_op<derived_t>()
		{
		}

		/**
		 * @destructor
		 */
		~redis_impl_t()
		{
			this->stop();
		}

		/** 
		 * @function : start the client, blocking connect to server 
		 * @param host A string identifying a location. May be a descriptive name or 
		 * a numeric address string. 
		 * @param port A string identifying the requested service. This may be a 
		 * descriptive name or a numeric string corresponding to a port number. 
		 */
		template <typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port) {
			return super::start(std::forward<String>(host), std::forward<StrOrInt>(port));
		}


		/** 
		 * @function : start the client, asynchronous connect to server 
		 * @param host A string identifying a location. May be a descriptive name or 
		 * a numeric address string. 
		 * @param port A string identifying the requested service. This may be a 
		 * descriptive name or a numeric string corresponding to a port number. 
		 */
		template <typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port) {
			// return this->derived().template _do_connect<true>(
			// 	std::forward<String>(host), std::forward<StrOrInt>(port));
			return super::async_start(std::forward<String>(host), std::forward<StrOrInt>(port));
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<const redis_reply&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}





	protected:
		template<class T>
		inline void _post_recv(std::shared_ptr<derived_t> this_ptr, const T&)
		{
			// std::cout<<__FUNCTION__<<std::endl;
			this->derived()._redis_post_recv(std::move(this_ptr));
		}

		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr)
		{
			// std::cout<<__FUNCTION__<<std::endl;
			this->derived()._redis_handle_recv(ec, bytes_recvd, std::move(this_ptr));
		}



		inline void _fire_recv(detail::ignore, redis_reply&& rep)
		{
			std::cout<<__FUNCTION__<<std::endl;
			this->listener_.notify(event_type::recv, rep);
		}




	protected:

		// std::list<std::function<void(const error_code& , redis_reply)>> callback_list_;
};
}

namespace asio2{
	class redis : public detail::redis_impl_t<redis, detail::template_args_redis>
	{
	public:
		public:
		using redis_impl_t<redis, detail::template_args_redis>::redis_impl_t;
	};
}


#if defined(ASIO2_USE_SSL)

#include <asio2/tcp/tcps_client.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class ssl_redis_impl_t
		: protected tcps_client_impl_t<derived_t, args_t>
		, public redis_send_cp<derived_t>
		, public redis_recv_op<derived_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT1;
		template <class>         friend class redis_send_cp;
		template <class>         friend class redis_recv_op;

	public:
		using self = ssl_redis_impl_t<derived_t, args_t>;
		using super = tcps_client_impl_t<derived_t, args_t>;

		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit ssl_redis_impl_t(
			std::size_t init_buffer_size = tcp_frame_size, 
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			Args&&... args
		)
			: super(init_buffer_size, max_buffer_size, args...)
			, redis_send_cp<derived_t>()
			, redis_recv_op<derived_t>()
		{
		}

		/**
		 * @destructor
		 */
		~ssl_redis_impl_t()
		{
			this->stop();
		}

		/** 
		 * @function : start the client, blocking connect to server 
		 * @param host A string identifying a location. May be a descriptive name or 
		 * a numeric address string. 
		 * @param port A string identifying the requested service. This may be a 
		 * descriptive name or a numeric string corresponding to a port number. 
		 */
		template <typename String, typename StrOrInt>
		bool start(String&& host, StrOrInt&& port) {
			return super::start(std::forward<String>(host), std::forward<StrOrInt>(port));
		}


		/** 
		 * @function : start the client, asynchronous connect to server 
		 * @param host A string identifying a location. May be a descriptive name or 
		 * a numeric address string. 
		 * @param port A string identifying the requested service. This may be a 
		 * descriptive name or a numeric string corresponding to a port number. 
		 */
		template <typename String, typename StrOrInt>
		bool async_start(String&& host, StrOrInt&& port) {
			// return this->derived().template _do_connect<true>(
			// 	std::forward<String>(host), std::forward<StrOrInt>(port));
			return super::async_start(std::forward<String>(host), std::forward<StrOrInt>(port));
		}

		/** 
		 * @function : stop the client 
		 * You can call this function on the communication thread and anywhere to stop the client. 
		 */
		inline void stop() {

			super::stop();
		}
	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::string_view s)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::recv,
				observer_t<const redis_reply&>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind connect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called after the client connection completed, whether successful or unsuccessful
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_connect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::connect, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind disconnect listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * This notification is called before the client is ready to disconnect
		 * Function signature : void(asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_disconnect(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::disconnect, observer_t<error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

		/**
		 * @function : bind init listener,we should set socket options at here
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void()
		 */
		template<class F, class ...C>
		inline derived_t & bind_init(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::init, observer_t<>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}


	protected:

		inline void _post_recv(std::shared_ptr<derived_t> this_ptr)
		{
			// std::cout<<__FUNCTION__<<std::endl;
			this->derived()._redis_post_recv(std::move(this_ptr));
		}


		inline void _handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr)
		{
			// std::cout<<__FUNCTION__<<std::endl;
			this->derived()._redis_handle_recv(ec, bytes_recvd, std::move(this_ptr));
		}

		inline void _fire_init()
		{
			this->listener_.notify(event_type::init);
		}

		inline void _fire_recv(detail::ignore, redis_reply&& rep)
		{
			std::cout<<__FUNCTION__<<std::endl;
			this->listener_.notify(event_type::recv, rep);
		}

		inline void _fire_connect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event_type::connect, ec);
		}

		inline void _fire_disconnect(detail::ignore, error_code ec)
		{
			this->listener_.notify(event_type::disconnect, ec);
		}


		/**
		 * @function : check whether the client is started
		 */
		inline bool is_started() const noexcept{
			return (this->state_ == state_t::started &&
					this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : check whether the client is stopped
		 */
		inline bool is_stopped() const noexcept{
			return (this->state_ == state_t::stopped &&
					!this->socket_.lowest_layer().is_open());
		}

		/**
		 * @function : get the buffer object refrence
		 */
		inline buffer_wrap<buffer_type>& buffer() noexcept{ return this->buffer_; }


	protected:
		/**
		 * @function : get the recv/read allocator object refrence
		 */
		inline auto& rallocator() noexcept{ return this->rallocator_; }
		/**
		 * @function : get the send/write allocator object refrence
		 */
		inline auto& wallocator() noexcept{ return this->wallocator_; }

		inline listener_t& listener() noexcept{ return this->listener_; }
		inline std::atomic<state_t>& state() noexcept{ return this->state_; }
		inline std::shared_ptr<derived_t> selfptr() { return std::shared_ptr<derived_t>{};}
		inline io_t& io(){ return this->io_; }

	protected:

		// std::list<std::function<void(const error_code& , redis_reply)>> callback_list_;
};
}

namespace asio2{
	class ssl_redis : public detail::ssl_redis_impl_t<ssl_redis, detail::template_args_ssl_redis>
	{
	public:
		using ssl_redis_impl_t<ssl_redis, detail::template_args_ssl_redis>::ssl_redis_impl_t;
	};
}


#endif

#endif // !__ASIO2_REDIS_CLIENT_HPP__
