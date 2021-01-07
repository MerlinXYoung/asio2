/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_TCP_SEND_OP_HPP__
#define __ASIO2_TCP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class tcp_send_op
	{
	protected:

		template<class, class = std::void_t<>>
		struct has_member_match_role_type : std::false_type {};

		template<class T>
		struct has_member_match_role_type<T, std::void_t<decltype(T::match_role_type_)>> : std::true_type {};

	public:
		/**
		 * @constructor
		 */
		tcp_send_op() = default;

		/**
		 * @destructor
		 */
		~tcp_send_op() = default;

	protected:
		template<class Data, class Callback>
		inline bool _tcp_send(Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (has_member_match_role_type<derived_t>::value)
			{
				//std::cout<<"match_role_type:"<<(int)derive.match_role_type_<<std::endl;
				switch (derive.match_role_type_)
				{
					case match_role_type::GENERAL:
					return derive._tcp_send_general(asio::buffer(data), std::forward<Callback>(callback));
					case match_role_type::DGRAM:
					return derive._tcp_send_dgram(asio::buffer(data), std::forward<Callback>(callback));
					case match_role_type::FIXED2:
					return derive._tcp_send_fixed2(asio::buffer(data), std::forward<Callback>(callback));
					case match_role_type::FIXED4:
					return derive._tcp_send_fixed4(asio::buffer(data), std::forward<Callback>(callback));
					default:
						assert(false);
				}
			}
			else
			{
				std::ignore = true;
			}

			return derive._tcp_send_general(asio::buffer(data), std::forward<Callback>(callback));
		}

		template<class BufferSequence, class Callback>
		inline bool _tcp_send_dgram(BufferSequence&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			int bytes = 0;
			std::unique_ptr<std::uint8_t[]> head;

			// note : need ensure big endian and little endian
			if (buffer.size() < std::size_t(254))
			{
				bytes = 1;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(buffer.size());
			}
			else if (buffer.size() <= (std::numeric_limits<std::uint16_t>::max)())
			{
				bytes = 3;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(254);
				std::uint16_t size = hton(static_cast<std::uint16_t>(buffer.size()));
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint16_t));
				// use little endian
				// if (!is_little_endian())
				// {
				// 	swap_bytes<sizeof(std::uint16_t)>(&head[1]);
				// }
				
			}
			else
			{
				ASIO2_ASSERT(buffer.size() > (std::numeric_limits<std::uint16_t>::max)());
				bytes = 9;
				head = std::make_unique<std::uint8_t[]>(bytes);
				head[0] = static_cast<std::uint8_t>(255);
				std::uint64_t size = hton<size_t>(buffer.size());
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint64_t));
				// use little endian
				// if (!is_little_endian())
				// {
				// 	swap_bytes<sizeof(std::uint64_t)>(&head[1]);
				// }
			}

			std::array<asio::const_buffer, 2> buffers
			{
				asio::buffer(reinterpret_cast<const void*>(head.get()), bytes),
				std::forward<BufferSequence>(buffer)
			};

			asio::async_write(derive.stream(), buffers, 
				make_allocator(derive.wallocator(),
					[&derive, p = derive.selfptr(),
					bytes, head = std::move(head),
					callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				if (ec)
				{
					callback(ec, bytes_sent);

					// must stop, otherwise re-sending will cause header confusion
					derive._do_disconnect(ec);
				}
				else
				{
					callback(ec, bytes_sent - bytes);
				}
			}));
			return true;
		}

		template<class BufferSequence, class Callback>
		inline bool _tcp_send_fixed2(BufferSequence&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);
			if(buffer.size() >= (std::numeric_limits<std::uint16_t>::max)())
			{
				return false;
			}
			std::unique_ptr<std::uint16_t> head = std::make_unique<std::uint16_t>();
			//std::cout<<"session:"<<this<<"send size:"<<buffer.size()<<std::endl;
			*head = hton<std::uint16_t>(buffer.size());

			//std::cout<<"session:"<<this<<"send size:"<<buffer.size()<<std::endl;

			std::array<asio::const_buffer, 2> buffers
			{
				asio::buffer(reinterpret_cast<const void*>(head.get()), sizeof(uint16_t)),
				std::forward<BufferSequence>(buffer)
			};

			asio::async_write(derive.stream(), buffers, 
				make_allocator(derive.wallocator(),
					[&derive, p = derive.selfptr(), head = std::move(head),
					callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				if (ec)
				{
					callback(ec, bytes_sent);

					// must stop, otherwise re-sending will cause header confusion
					derive._do_disconnect(ec);
				}
				else
				{
					callback(ec, bytes_sent - sizeof(uint16_t));
				}
			}));
			return true;
		}

		template<class BufferSequence, class Callback>
		inline bool _tcp_send_fixed4(BufferSequence&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);
			if(buffer.size() >= (std::numeric_limits<std::uint32_t>::max)())
			{
				return false;
			}
			std::unique_ptr<std::uint32_t> head = std::make_unique<std::uint32_t>();
			*head = hton<std::uint32_t>(buffer.size());

			std::array<asio::const_buffer, 2> buffers
			{
				asio::buffer(reinterpret_cast<const void*>(head.get()), sizeof(uint32_t)),
				std::forward<BufferSequence>(buffer)
			};

			asio::async_write(derive.stream(), buffers, 
				make_allocator(derive.wallocator(),
					[&derive, p = derive.selfptr(),head = std::move(head),
					callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				if (ec)
				{
					callback(ec, bytes_sent);

					// must stop, otherwise re-sending will cause header confusion
					derive._do_disconnect(ec);
				}
				else
				{
					callback(ec, bytes_sent - sizeof(uint32_t));
				}
			}));
		}

		template<class BufferSequence, class Callback>
		inline bool _tcp_send_general(BufferSequence&& buffer, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::async_write(derive.stream(), buffer, make_allocator(derive.wallocator(),
					[&derive, p = derive.selfptr(), callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				callback(ec, bytes_sent);

				if (ec)
				{
					// must stop, otherwise re-sending will cause body confusion
					derive._do_disconnect(ec);
				}
			}));
			return true;
		}

	protected:
	};
}

#endif // !__ASIO2_TCP_SEND_OP_HPP__
