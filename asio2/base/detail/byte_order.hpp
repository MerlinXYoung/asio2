#pragma once

#include <cstdint>
namespace asio2::detail{
template<class T>
inline T hton(T);
template<class T>
inline T ntoh(T);
}
#ifdef ASIO2_SWAP_BYTES
namespace asio2::detail{
    // Returns true if the current machine is little endian
	inline bool is_little_endian()
	{
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(&test) == 1);
	}

	template<class T>
	void swap(T& a, T& b){
		a^=b;
		b^=a;
		a^=b;
	}
	/**
	 * Swaps the order of bytes for some chunk of memory (reverse_bytes)
	 * @param data The data as a uint8_t pointer
	 * @tparam DataSize The true size of the data
	 */
	template <std::size_t DataSize>
	inline void swap_bytes(std::uint8_t * data)
	{
		for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
			std::swap(data[i], data[DataSize - i - 1]);
	}
	template <>
	inline void swap_bytes<2>(std::uint8_t * data)
	{
			std::swap(data[0], data[1]);
	}
	template <>
	inline void swap_bytes<4>(std::uint8_t * data)
	{
			std::swap(data[0], data[3]);
			std::swap(data[1], data[2]);
	}
	template <>
	inline void swap_bytes<8>(std::uint8_t * data)
	{
			std::swap(data[0], data[7]);
			std::swap(data[1], data[6]);
			std::swap(data[2], data[5]);
			std::swap(data[3], data[4]);
	}

	template<class T>
	inline 
	typename std::enable_if<std::is_integral<T>::value, T>::type hton(T s){
		if(is_little_endian()){
			swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t*>(&s));
		}
		return s;
	}

	template<class T>
	inline 
	typename std::enable_if<std::is_integral<T>::value, T>::type ntoh(T s){
		if(is_little_endian()){
			swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t*>(&s));
		}
		return s;
	}
}
#else
#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")

template<>
inline uint16_t hton<uint16_t>(uint16_t v) {
    return htons(v);
}
template<>
inline uint32_t hton<uint32_t>(uint32_t v) {
    return htonl(v);
}
template<>
inline uint64_t hton<uint64_t>(uint64_t v) {
    return htonll(v);
}
template<>
inline uint16_t ntoh<uint16_t>(uint16_t v) {
    return ntohs(v);
}
template<>
inline uint32_t ntoh<uint32_t>(uint32_t v) {
    return ntohl(v);
}
template<>
inline uint64_t ntoh<uint64_t>(uint64_t v) {
    return ntohll(v);
}

#else

#include <stdint.h>

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>

#define htobe16 OSSwapHostToBigInt16
#define htobe32 OSSwapHostToBigInt32
#define htobe64 OSSwapHostToBigInt64
#define be16toh OSSwapBigToHostInt16
#define be32toh OSSwapBigToHostInt32
#define be64toh OSSwapBigToHostInt64

#elif defined(__linux__)
#include <endian.h>
#else
#include <sys/endian.h>
#endif
namespace asio2::detail{
template<>
inline uint16_t hton<uint16_t>(uint16_t v) {
    return htobe16(v);
}
template<>
inline uint32_t hton<uint32_t>(uint32_t v) {
    return htobe32(v);
}
template<>
inline uint64_t hton<uint64_t>(uint64_t v) {
    return htobe64(v);
}
template<>
inline uint16_t ntoh<uint16_t>(uint16_t v) {
    return be16toh(v);
}
template<>
inline uint32_t ntoh<uint32_t>(uint32_t v) {
    return be32toh(v);
}
template<>
inline uint64_t ntoh<uint64_t>(uint64_t v) {
    return be64toh(v);
}
}
#endif
#endif