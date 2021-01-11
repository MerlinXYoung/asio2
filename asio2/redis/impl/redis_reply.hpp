#ifndef __ASIO2_REDIS_VALUE_HPP__
#define __ASIO2_REDIS_VALUE_HPP__


#include <vector>
#include <string>
#include <variant>
#include <type_traits>
#include <asio2/base/selector.hpp>
namespace asio2::detail{

class redis_reply {
private:
    struct null_tag {
        inline bool operator == (const null_tag &) const {
            return true;
        }
    };
    
public:
    using error_type=std::string;
    using bytes_type=std::vector<char>;
    using array_type=std::vector<redis_reply>;
    using int_type=std::int64_t;
    using value_type = typename std::variant<null_tag, int_type, error_type, bytes_type, array_type >;
    redis_reply(): value_(null_tag())
    {
    }

    redis_reply(redis_reply &&other): value_(std::move(other.value_))
    {
        std::cout<<"copy redis_reply:"<<to_string()<<std::endl;
    }

    redis_reply(int_type i): value_(i)
    {
    }

    redis_reply(error_type s): value_( std::move(s))
    {
    }
    // template<bool isError=false>
    // redis_reply(bytes_type bytes): value_(isError?std::string(bytes.begin(),bytes.end()):std::move(bytes))
    // {
    // }

    redis_reply(bytes_type bytes): value_(std::move(bytes))
    {
    }

    redis_reply(array_type array): value_(std::move(array))
    {
        std::cout<<__FUNCTION__<<":"<<get<array_type>().size()<<std::endl;
    }

    redis_reply(const redis_reply &) = delete;
    redis_reply& operator = (const redis_reply &) = delete;
    redis_reply& operator = (redis_reply && r){ 
        
        value_ = std::move(r.value_);
        std::cout<<"copy redis_reply = "<<to_string()<<std::endl;
    }

    ~redis_reply(){ 
        std::cout<<"~"<<to_string()<<std::endl;
    }

    // template<class T>
    // inline redis_reply& operator = (const T& v){
    //     std::swap(value_, redis_reply(v));
    //     return *this;
    // }

    template<class T>
    inline redis_reply& operator = (T&& v){
        value_ = std::move(v);
        std::cout<<"="<<to_string()<<std::endl;
        // value_type tmp(v);
        // std::swap(value_, tmp);
        return *this;
    }

    template<class T>
    inline redis_reply& emplace(T&& v){
        value_.emplace<T>(std::move(v));
        return *this;
    }

    // inline redis_reply& operator = (int_type i){
    //     value_=i;
    // }
    // inline redis_reply& operator = (const error_type& e){
    //     value_=e;
    // }
    // inline redis_reply& operator = (const bytes_type& byes){
    //     value_=bytes;
    // }
    // inline redis_reply& operator = (const array_type& a){
    //     value_=a;
    // }

    // inline redis_reply& operator = (error_type&& e){
    //     value_=e;
    // }
    // inline redis_reply& operator = (bytes_type&& byes){
    //     value_=bytes;
    // }
    // inline redis_reply& operator = (array_type&& a){
    //     value_=a;
    // }

    // Return the redis_reply as a std::string if
    // type is a byte string; otherwise returns an empty std::string.
#if 0
    std::string to_string() const
    {
        // std::string str;
        return std::visit([/*&str*/](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, redis_reply::int_type>)
                return std::to_string(arg);
            else if constexpr (std::is_same<T, redis_reply::error_type>::value)
                return arg;
            else if constexpr (std::is_same<T, redis_reply::bytes_type>::value)
                return std::string(arg.begin(), arg.end());
            else if constexpr (std::is_same<T, redis_reply::null_tag>::value)
                return "(null)";
            else 
                return "";
        }, value_);
    }
#else 
    std::string to_string()const{ 
        if( is_error() )
        {
            static std::string err = "error: ";
            std::string result;

            result = err;
            result += get<error_type>();

            return result;
        }
        else if( is_null() )
        {
            static std::string null = "(null)";
            return null;
        }
        else if( is_int() )
        {
            return std::to_string(get<int_type>());
        }
        else if( is_bytes() )
        {
            const auto& bytes = get<bytes_type>();
            return std::string(bytes.begin(), bytes.end());
        }
        else if( is_array())
        {
            const auto& array = get<array_type>();
            std::string str("[");
            for(auto& rep: array)
            {
                str+=rep.to_string();
                str+=",";
            }
            if(str.back()==',')
                str.back() = ']' ; 
            else   
                str += ']';
            return str;
        }
        else
            return "{}";
    }
#endif


    // Return the string representation of the redis_reply. Use
    // for dump content of the redis_reply.
    // std::string inspect() const;

    // Return true if redis_reply not a error
    inline bool is_ok() const
    {
        return !is_error();
    }
    // Return true if redis_reply is a error
    inline bool is_error() const
    {
        return std::holds_alternative<std::string>(value_);
    }
    // Return true if this is a null.
    inline bool is_null() const
    {
        return std::holds_alternative<null_tag>(value_);
    }
    // Return true if type is an int
    inline bool is_int() const
    {
        return std::holds_alternative<int64_t>(value_);
    }
    
    // Return true if type is a string/byte array. Alias for isString();
    inline bool is_string() const
    {
        return std::holds_alternative<std::vector<char> >(value_) 
            || std::holds_alternative<std::string >(value_);
    }
    // Return true if type is a string/byte array. Alias for isByteArray().
    inline bool is_bytes() const
    {
        return std::holds_alternative<std::vector<char> >(value_) 
            || std::holds_alternative<std::string >(value_);
    }
    // Return true if type is an array
    inline bool is_array() const
    {
        return std::holds_alternative< std::vector<redis_reply> >(value_);
    }
    // Return true if is public message
    inline bool is_message()const{ 
        if(!is_array()) 
            return false; 
        const auto& array = get<array_type>();
        if(array.empty())
            return false;
        const auto& bytes = array.begin()->get<bytes_type>();
        return bytes.size() == 7LU && strncmp(bytes.data(), "message", bytes.size()) == 0;

    }

    template<class T>
    inline T& get(){
        return std::get<T>(value_);
    }
    template<class T>
    inline const T& get() const{
        return std::get<T>(value_);
    }

    template<class T>
    inline T* get_if(){
        return std::get_if<T>(value_);
    }
    template<class T>
    inline const T* get_if() const{
        return std::get_if<T>(value_);
    }

    //  std::vector<char> &get_bytes();
    //  const std::vector<char> &get_bytes() const;
    //  std::vector<redis_reply> &get_array();
    //  const std::vector<redis_reply> &get_array() const;


    inline bool operator == (const redis_reply &rhs) const
    {
        return value_ == rhs.value_;
    }

    inline bool operator != (const redis_reply &rhs) const
    {
        return !(*this == rhs);
    }


public:
    inline void clear()
    {
        value_ = value_type(null_tag());
    }
private:
    value_type value_;
};
#if 0
std::string redis_reply::inspect() const
{
    if( is_error() )
    {
        static std::string err = "error: ";
        std::string result;

        result = err;
        result += get<error_type>();

        return result;
    }
    else if( is_null() )
    {
        static std::string null = "(null)";
        return null;
    }
    else if( is_int() )
    {
        return std::to_string(get<int_type>());
    }
    else if( is_bytes() )
    {
        const auto& bytes = get<bytes_type>();
        return std::string(bytes.begin(), bytes.end());
    }
    else if( is_array())
    {
        const auto& values = this->get<array_type>();
        std::cout<<"values count:"<<values.size()<<std::endl;
        std::string result = "[";
        for(const auto& v :values)
        {
            result += v.inspect();
            result += ", ";
        }
        if(result.back()==',')
           result.back() = ']' ; 
        else   
            result += ']';
        return result;
    }
    else{
        return "fuck";
    }
}
#endif

}

namespace asio2{
    using redis_reply = detail::redis_reply;
}


#endif//__ASIO2_REDIS_VALUE_HPP__