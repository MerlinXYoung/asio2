
#ifndef __ASIO2_REDIS_REPLY_PARSER_HPP__
#define __ASIO2_REDIS_REPLY_PARSER_HPP__

#include <stack>
#include <vector>
#include <utility>

#include <asio2/redis/impl/redis_reply.hpp>


namespace asio2 {
namespace detail{
class redis_reply_parser
{
public:
    redis_reply_parser();

    enum ParseResult {
        Completed,
        Incompleted,
        Error,
    };

     std::pair<size_t, ParseResult> parse(const char *ptr, size_t size);

     redis_reply& result(){ return reply_; }
    const redis_reply& result()const { return reply_; }
protected:
    void reset()
    {
        auto states(std::move(states_));

        bulk_size_=0;
        buf_.clear();
        reply_.clear();

        // temporary variables
        auto array_sizes(std::move(array_sizes_));
        auto array_reply(std::move(array_reply_));
    }
     std::pair<size_t, ParseResult> parse_chunk(const char *ptr, size_t size);

    inline bool is_char(int c)
    {
        return c >= 0 && c <= 127;
    }

    inline bool is_control(int c)
    {
        return (c >= 0 && c <= 31) || (c == 127);
    }

     long int buf_to_long(const char *str, size_t size);

private:
    enum State {
        Start = 0,
        StartArray = 1,

        String = 2,
        StringLF = 3,

        ErrorString = 4,
        ErrorLF = 5,

        Integer = 6,
        IntegerLF = 7,

        BulkSize = 8,
        BulkSizeLF = 9,
        Bulk = 10,
        BulkCR = 11,
        BulkLF = 12,

        ArraySize = 13,
        ArraySizeLF = 14,
    };

    std::stack<State> states_;

    long int bulk_size_;
    std::vector<char> buf_;
    redis_reply reply_;

    // temporary variables
    std::stack<long int> array_sizes_;
    std::stack<redis_reply> array_reply_;

    static const char string_reply = '+';
    static const char error_reply = '-';
    static const char integer_reply = ':';
    static const char bulk_reply = '$';
    static const char array_reply = '*';
};
redis_reply_parser::redis_reply_parser()
    : bulk_size_(0)
{
    buf_.reserve(64);
}

std::pair<size_t, redis_reply_parser::ParseResult> redis_reply_parser::parse(const char *ptr, size_t size)
{
    return redis_reply_parser::parse_chunk(ptr, size);
}

std::pair<size_t, redis_reply_parser::ParseResult> redis_reply_parser::parse_chunk(const char *ptr, size_t size)
{
    size_t position = 0;
    State state = Start;

    if (!states_.empty())
    {
        state = states_.top();
        states_.pop();
    }

    while(position < size)
    {
        char c = ptr[position++];
#ifdef DEBUG_REDIS_PARSER
        std::cerr << "state: " << state << ", c: " << c << "\n";
#endif

        switch(state)
        {
            case StartArray:
            case Start:
                buf_.clear();
                switch(c)
                {
                    case string_reply:
                        state = String;
                        break;
                    case error_reply:
                        state = ErrorString;
                        break;
                    case integer_reply:
                        state = Integer;
                        break;
                    case bulk_reply:
                        state = BulkSize;
                        bulk_size_ = 0;
                        break;
                    case array_reply:
                        state = ArraySize;
                        break;
                    default:
                        return std::make_pair(position, Error);
                }
                break;
            case String:
                if( c == '\r' )
                {
                    state = StringLF;
                }
                else if( is_char(c) && !is_control(c) )
                {
                    buf_.push_back(c);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case ErrorString:
                if( c == '\r' )
                {
                    state = ErrorLF;
                }
                else if( is_char(c) && !is_control(c) )
                {
                    buf_.push_back(c);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case BulkSize:
                if( c == '\r' )
                {
                    if( buf_.empty() )
                    {
                        std::stack<State>().swap(states_);
                        return std::make_pair(position, Error);
                    }
                    else
                    {
                        state = BulkSizeLF;
                    }
                }
                else if( isdigit(c) || c == '-' )
                {
                    buf_.push_back(c);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case StringLF:
                if( c == '\n')
                {
                    state = Start;
                    reply_ = std::move(buf_);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case ErrorLF:
                if( c == '\n')
                {
                    state = Start;
                    reply_ = std::move(std::string(buf_.begin(), buf_.end()));
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case BulkSizeLF:
                if( c == '\n' )
                {
                    bulk_size_ = buf_to_long(buf_.data(), buf_.size());
                    buf_.clear();

                    if( bulk_size_ == -1 )
                    {
                        state = Start;
                        reply_.clear(); // Nil
                    }
                    else if( bulk_size_ == 0 )
                    {
                        state = BulkCR;
                    }
                    else if( bulk_size_ < 0 )
                    {
                        std::stack<State>().swap(states_);
                        return std::make_pair(position, Error);
                    }
                    else
                    {
                        buf_.reserve(bulk_size_);

                        long int available = size - position;
                        long int canRead = std::min(bulk_size_, available);

                        if( canRead > 0 )
                        {
                            buf_.assign(ptr + position, ptr + position + canRead);
                            position += canRead;
                            bulk_size_ -= canRead;
                        }


                        if (bulk_size_ > 0)
                        {
                            state = Bulk;
                        }
                        else
                        {
                            state = BulkCR;
                        }
                    }
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case Bulk: {
                assert( bulk_size_ > 0 );

                long int available = size - position + 1;
                long int canRead = std::min(available, bulk_size_);

                buf_.insert(buf_.end(), ptr + position - 1, ptr + position - 1 + canRead);
                bulk_size_ -= canRead;
                position += canRead - 1;

                if( bulk_size_ == 0 )
                {
                    state = BulkCR;
                }
                break;
            }
            case BulkCR:
                if( c == '\r')
                {
                    state = BulkLF;
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case BulkLF:
                if( c == '\n')
                {
                    state = Start;
                    reply_ = std::move(buf_);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case ArraySize:
                if( c == '\r' )
                {
                    if( buf_.empty() )
                    {
                        std::stack<State>().swap(states_);
                        return std::make_pair(position, Error);
                    }
                    else
                    {
                        state = ArraySizeLF;
                    }
                }
                else if( isdigit(c) || c == '-' )
                {
                    buf_.push_back(c);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case ArraySizeLF:
                if( c == '\n' )
                {
                    int64_t arraySize = buf_to_long(buf_.data(), buf_.size());
                    // std::cout<<"array size:"<<arraySize<<std::endl;
                    std::vector<redis_reply> array;

                    if( arraySize == -1 )
                    {
                        state = Start;
                        reply_.clear();  // Nil value
                    }
                    else if( arraySize == 0 )
                    {
                        state = Start;
                        // reply_.operator=(std::move(array));  // Empty array
                        reply_.emplace<std::vector<redis_reply>>(std::move(array));
                    }
                    else if( arraySize < 0 )
                    {
                        std::stack<State>().swap(states_);
                        return std::make_pair(position, Error);
                    }
                    else
                    {
                        array.reserve(arraySize);
                        array_sizes_.push(arraySize);
                        array_reply_.push(std::move(array));

                        state = StartArray;
                    }
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case Integer:
                if( c == '\r' )
                {
                    if( buf_.empty() )
                    {
                        std::stack<State>().swap(states_);
                        return std::make_pair(position, Error);
                    }
                    else
                    {
                        state = IntegerLF;
                    }
                }
                else if( isdigit(c) || c == '-' )
                {
                    buf_.push_back(c);
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            case IntegerLF:
                if( c == '\n' )
                {
                    int64_t value = buf_to_long(buf_.data(), buf_.size());

                    buf_.clear();
                    reply_ = value;
                    state = Start;
                }
                else
                {
                    std::stack<State>().swap(states_);
                    return std::make_pair(position, Error);
                }
                break;
            default:
                std::stack<State>().swap(states_);
                return std::make_pair(position, Error);
        }


        if (state == Start)
        {
            if (!array_sizes_.empty())
            {
                assert(array_sizes_.size() > 0);
                // std::cout<<"pushback"<<std::endl;
                array_reply_.top().get<redis_reply::array_type>().emplace_back(std::move(reply_));

                while(!array_sizes_.empty() && --array_sizes_.top() == 0)
                {
                    array_sizes_.pop();
                    // std::cout<<"array ok:"<<array_reply_.top().get<redis_reply::array_type>().size()<<std::endl;
                    reply_ = std::move(array_reply_.top());
                    // std::cout<<"array ok:"<<reply_.get<redis_reply::array_type>().size()<<std::endl;
                    array_reply_.pop();

                    if (!array_sizes_.empty())
                        array_reply_.top().get<redis_reply::array_type>().emplace_back(std::move(reply_));
                }
            }


            if (array_sizes_.empty())
            {
                // done
                break;
            }
        }
    }
// std::cout<<"array ok:"<<reply_.get<redis_reply::array_type>().size()<<std::endl;
    // std::cout<<reply_.to_string()<<std::endl;
    if (array_sizes_.empty() && state == Start)
    {
        return std::make_pair(position, Completed);
    }
    else
    {
        states_.push(state);
        return std::make_pair(position, Incompleted);
    }
    // std::cout<<"array ok:"<<reply_.get<redis_reply::array_type>().size()<<std::endl;
}

// redis_reply redis_reply_parser::result()
// {
//     return std::move(reply_);
// }

/*
 * Convert string to long. I can't use atol/strtol because it
 * work only with null terminated string. I can use temporary
 * std::string object but that is slower then buf_to_long.
 */
long int redis_reply_parser::buf_to_long(const char *str, size_t size)
{
    long int value = 0;
    bool sign = false;

    if( str == nullptr || size == 0 )
    {
        return 0;
    }

    if( *str == '-' )
    {
        sign = true;
        ++str;
        --size;

        if( size == 0 ) {
            return 0;
        }
    }

    for(const char *end = str + size; str != end; ++str)
    {
        char c = *str;

        // char must be valid, already checked in the parser
        assert(c >= '0' && c <= '9');

        value = value * 10;
        value += c - '0';
    }

    return sign ? -value : value;
}
}
}


#endif // __ASIO2_REDIS_REPLY_PARSER_HPP__
