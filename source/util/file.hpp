#pragma once
#include "path.hpp"
#include <string>

namespace ss
{
	namespace util
	{
		inline namespace file
		{
			// Read whole file into byte string (may contains zero)
			std::string slurp(Sanitized_Runtime_Path const& path);
		}
	}
}

