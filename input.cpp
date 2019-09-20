#include "input.h"
#include <memory>

#include <readline/readline.h>

namespace mgo
{

std::string input( const std::string& prompt )
{
    auto deleter = [](char * c){ free(c); };
    std::unique_ptr<char, decltype(deleter)> p(
        ::readline( prompt.c_str() ),
        deleter
        );
    return std::string(p.get());
}

} // end namespace