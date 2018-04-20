#pragma once
#include "../../util/path.hpp"
#include "../../util/file.hpp"
#include "../../util/string.hpp"

namespace path_util
{
	using ss::util::is_absolute_path;
	using ss::util::generate_cubemap_paths;
	using ss::util::Sanitized_Runtime_Path;
	using ss::util::Working_Directory_File_Guard;
}

namespace file_util
{
	using ss::util::slurp;
}

namespace string_util
{
	using ss::util::starts_with;
}

