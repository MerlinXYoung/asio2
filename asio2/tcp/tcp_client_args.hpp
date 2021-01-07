#ifndef __ASIO2_TCP_CLIENT_ARGS_HPP__
#define __ASIO2_TCP_CLIENT_ARGS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string_view>
#include <asio2/config.hpp>



namespace asio2::detail
{
	struct template_args_tcp_client
	{
		static constexpr bool is_session = false;
		static constexpr bool is_client  = true;

		using socket_t    = asio::ip::tcp::socket;
		using buffer_t    = asio::streambuf;
		using send_data_t = std::string_view;
		using recv_data_t = std::string_view;
	};
}
#endif// __ASIO2_TCP_CLIENT_ARGS_HPP__
