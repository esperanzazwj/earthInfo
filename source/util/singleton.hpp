#pragma once
#include "constraint.hpp"

namespace ss
{
    namespace util
    {
        template <class Instance>
        struct Singleton: util::Non_Transferable
        {
            using instance_type = Instance;

            static instance_type& instance()
            {
                static instance_type inst;
                return inst;
            }

        protected:
            Singleton() = default;
        };
    }
}

