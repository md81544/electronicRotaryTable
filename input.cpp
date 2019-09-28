#include "input.h"
#include <memory>

#include <readline/readline.h>

namespace
{

char* defaultText = nullptr;

// This is a callback function, hence the separate char*
// which contains the default
int setDefault ()
{
  if ( defaultText )
    {
      /* Apparently the "current cursor position" in which text is inserted
         is 0, when initially called */
      rl_insert_text( defaultText );
      defaultText = nullptr;

      /* disable the global 'rl_startup_hook' function by setting it to NULL */
      rl_startup_hook = (rl_hook_func_t *) nullptr;
    }
  return 0;
}


} // end anonymous namespace

namespace mgo
{

std::string input(
    const std::string& prompt,
    const std::string& defaultValue /* = "" */
    )
{
    if(! defaultValue.empty() )
    {
        defaultText = const_cast<char*>( defaultValue.c_str() );
        rl_startup_hook = setDefault;
    }
    auto deleter = [](char * c){ free(c); };
    std::unique_ptr<char, decltype(deleter)> p(
        ::readline( prompt.c_str() ),
        deleter
        );
    return std::string(p.get());
}

} // end namespace