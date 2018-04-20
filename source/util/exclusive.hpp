#pragma once
// `exclusive` declares that a class can only have 1 instance at a time.
// Previous instance must be destroyed before the new one can be created.
// Current instance may be queried.
#include "constraint.hpp"
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <utility>      // for std::move

namespace ss
{
    namespace util
    {
        template <class Instance>
        struct Exclusive: util::Non_Transferable
        {
            using instance_type = Instance;

            Exclusive()
            {
                if (instance != nullptr) {
                    auto message = std::string{typeid(instance_type).name()};
                    message += " is exclusive. You must destroy the previous instance before creating a new one.";
                    throw std::logic_error{std::move(message)};
                }

                instance = static_cast<instance_type*>(this);
            }

            ~Exclusive()
            {
                instance = nullptr;
            }

            static instance_type& current()
            {
                if (instance == nullptr) {
                    auto message = std::string{typeid(instance_type).name()};
                    message += " has no instance. Please create one before calling current().";
                    throw std::logic_error{std::move(message)};
                }
                return *instance;
            }

        private:
            static instance_type* instance;
        };

        template <class Instance>
        typename Exclusive<Instance>::instance_type*
        Exclusive<Instance>::instance = nullptr;
    }
}

