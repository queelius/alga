#pragma once

#include <string>
#include "exp.hpp"

namespace aperture::detail
{
    struct symbol : exp
    {
        std::string value;
    };
}