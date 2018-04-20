#pragma once
#include "../util/constraint.hpp"
#include <functional>
#include <vector>
#include <utility>      // for std::move

namespace ss
{
    inline namespace wsi
    {
        template <class ...Callback_Args>
        struct Event_Source final: util::Non_Transferable
        {
            using callback_fn_type = void (Callback_Args...);
            using callback_type = std::function<callback_fn_type>;
            using callback_list_type = std::vector<callback_type>;

            void subscribe(callback_type cb)
            {
                callbacks.emplace_back(std::move(cb));
            }

            void emit(Callback_Args... args)
            {
                for (auto& cb: callbacks)
                    cb(args...);
            }

            void operator () (Callback_Args... args) { emit(args...); }

        private:
            callback_list_type callbacks;
        };

        template <class Tag, class ...Callback_Args>
        struct Static_Event_Source final
        {
            using tag_type = Tag;
            using source_type = Event_Source<Callback_Args...>;
            using callback_type = typename source_type::callback_type;

            Static_Event_Source() = delete;

            static void emit(Callback_Args... args) { source.emit(args...); }
            static void subscribe(callback_type cb) { source.subscribe(std::move(cb)); }

        private:
            static source_type source;
        };

        template <class Tag, class ...Callback_Args>
        typename Static_Event_Source<Tag, Callback_Args...>::source_type
        Static_Event_Source<Tag, Callback_Args...>::source;
    }
}

