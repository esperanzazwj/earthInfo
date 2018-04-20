#include "registry.hpp"
#include <stdexcept>

namespace ss
{
    namespace app
    {
        auto Registry::metadata_of(std::string const& name) const -> metadata_type const*
        {
            auto it = metadatas.find(name);
            if (it == end(metadatas)) return nullptr;
            return &it->second;
        }

        auto Registry::collect_names() const -> std::vector<std::string>
        {
            std::vector<std::string> names;
            names.reserve(metadatas.size());

            for (auto& entry: metadatas)
                names.emplace_back(entry.first);

            return names;
        }

        void Registry::add(metadata_type md)
        {
            auto name = md.name;    // copying intended

            if (metadatas.count(name))
                throw std::logic_error{"Duplicate apps: " + name};

            metadatas.emplace(std::move(name), std::move(md));
        }
    }
}

