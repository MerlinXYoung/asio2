/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

#ifndef __ASIO2_HTTPS_SERVER_HPP__
#define __ASIO2_HTTPS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcps_server.hpp>
#include <asio2/http/https_session.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;

	template<class derived_t, class session_t>
	class https_server_impl_t
		: public tcps_server_impl_t<derived_t, session_t>
		, public http_router_t     <session_t           >
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;

	public:
		using super = tcps_server_impl_t <derived_t, session_t>;
		using self  = https_server_impl_t<derived_t, session_t>;

		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit https_server_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			std::size_t concurrency = std::thread::hardware_concurrency() * 2
		)
			: super(method, init_buffer_size, max_buffer_size, concurrency)
		{
		}

		/**
		 * @destructor
		 */
		~https_server_impl_t()
		{
			this->stop();
		}

		/**
		 * @function : start the server
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename StrOrInt>
		inline bool start(StrOrInt&& service)
		{
			return this->start(std::string_view{}, std::forward<StrOrInt>(service));
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt>
		inline bool start(String&& host, StrOrInt&& service)
		{
			return this->derived()._do_start(std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<void>{});
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename ParserFun>
		inline bool start(String&& host, StrOrInt&& service, ParserFun&& parser)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<ParserFun>>>;
			using IdT = typename fun_traits_type::return_type;
			using SendDataT = typename fun_traits_type::template args<0>::type;
			using RecvDataT = typename fun_traits_type::template args<0>::type;

			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<use_rdc_t<void, IdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<ParserFun>(parser)));
		}

		/**
		 * @function : start the server
		 * @param host A string identifying a location. May be a descriptive name or
		 * a numeric address string.
		 * @param service A string identifying the requested service. This may be a
		 * descriptive name or a numeric string corresponding to a port number.
		 */
		template<typename String, typename StrOrInt, typename SendParserFun, typename RecvParserFun>
		inline bool start(String&& host, StrOrInt&& service, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
		{
			using send_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<SendParserFun>>>;
			using recv_fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<RecvParserFun>>>;
			using SendIdT = typename send_fun_traits_type::return_type;
			using RecvIdT = typename recv_fun_traits_type::return_type;
			using SendDataT = typename send_fun_traits_type::template args<0>::type;
			using RecvDataT = typename recv_fun_traits_type::template args<0>::type;

			static_assert(std::is_same_v<SendIdT, RecvIdT>);

			return this->derived()._do_start(
				std::forward<String>(host), std::forward<StrOrInt>(service),
				condition_wrap<use_rdc_t<void, SendIdT, SendDataT, RecvDataT>>(
					std::in_place,
					std::forward<SendParserFun>(send_parser),
					std::forward<RecvParserFun>(recv_parser)));
		}

	public:
		/**
		 * @function : bind recv listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::https_session>& session_ptr,
		 *                           http::request& req, http::response& rep)
		 * or                 : void(http::request& req, http::response& rep)
		 */
		template<class F, class ...C>
		inline derived_t & bind_recv(F&& fun, C&&... obj)
		{
			if constexpr (is_template_callable_v<F,
				std::shared_ptr<session_t>&, http::request&, http::response&>)
			{
				this->is_arg0_session_ = true;
				this->listener_.bind(event_type::recv, observer_t<std::shared_ptr<session_t>&,
					http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			else
			{
				this->is_arg0_session_ = false;
				this->listener_.bind(event_type::recv, observer_t<
					http::request&, http::response&>(std::forward<F>(fun), std::forward<C>(obj)...));
			}
			return (this->derived());
		}

		/**
		 * @function : bind websocket upgrade listener
		 * @param    : fun - a user defined callback function
		 * Function signature : void(std::shared_ptr<asio2::http_session>& session_ptr, asio::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_upgrade(F&& fun, C&&... obj)
		{
			this->listener_.bind(event_type::upgrade, observer_t<std::shared_ptr<session_t>&, error_code>
				(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return super::_make_session(std::forward<Args>(args)..., *this,
				this->root_directory_, this->is_arg0_session_, this->support_websocket_);
		}

	protected:
		bool                      is_arg0_session_    = false;
	};
}

namespace asio2
{
	template<class session_t>
	class https_server_t : public detail::https_server_impl_t<https_server_t<session_t>, session_t>
	{
	public:
		using detail::https_server_impl_t<https_server_t<session_t>, session_t>::https_server_impl_t;
	};

	using https_server = https_server_t<https_session>;
}

#endif // !__ASIO2_HTTPS_SERVER_HPP__

#endif
