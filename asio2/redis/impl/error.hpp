#pragma once
#include <asio2/base/selector.hpp>

namespace asio2{
enum class redis_errors: int{
            ok=0,
            parse_failure=1,
};
class redis_error_category: public asio::error_category{
public:

    virtual const char* name() const noexcept override final
    {return "redis_error";}
    virtual std::string message(int v) const override final
    {
            switch (v) {
            case (int)redis_errors::ok:
                    return "(redis)OK";
            case (int)redis_errors::parse_failure:
                    return "(redis)parse_failure";
            default:                                                                                                                        return "";
            }
            return "";
    }
};

}
namespace std {
    template<> struct is_error_code_enum<asio2::redis_errors>{
            static const bool value = true;
    };
}

namespace asio2{
    inline const asio::error_category& get_redis_error_category(){
            static redis_error_category obj;    
            return obj;
    }
    inline asio::error_code make_error_code(redis_errors code){
            return error_code((int)code, get_redis_error_category());
    }
    inline void throw_error(redis_errors code, const char* what = ""){
            if (std::uncaught_exception())
                    return ;
            throw asio2::system_error(make_error_code(code), what);
    }
}



