#pragma once

#include <array>
#include <string>

namespace grimm
{
    typedef std::array<std::string, 2048> Dictionary;

    namespace language
    {
        extern const Dictionary en;
        extern const Dictionary es;
        extern const Dictionary ja;
        extern const Dictionary it;
        extern const Dictionary fr;
        extern const Dictionary cs;
        extern const Dictionary ru;
        extern const Dictionary uk;
        extern const Dictionary zh_Hans;
        extern const Dictionary zh_Hant;
    }
}