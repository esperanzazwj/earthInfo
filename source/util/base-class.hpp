#pragma once
// Declares a class to be a base class.
#include "constraint.hpp"

namespace ss
{
    namespace util
    {
        struct Base_Class: util::Non_Transferable
        {
            virtual ~Base_Class() {}
        };
    }
}

