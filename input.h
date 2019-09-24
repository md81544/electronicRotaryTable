#pragma once

#include <string>

namespace mgo
{

// Thin wrapper around the readline library
std::string input(
    const std::string& prompt,
    const std::string& defaultValue = ""
    );

} // end namespace
