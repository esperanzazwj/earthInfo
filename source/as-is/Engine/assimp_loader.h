#pragma once
#include "../../util/path.hpp"
#include "engine_struct.h"
#include <string>

bool Import3DFromFile(
    ss::util::Sanitized_Runtime_Path const& filepath,
    std::vector<as_Mesh*>& meshs,
    std::vector<as_Geometry*>& geo,
    std::vector<as_Skeleton*>& skeletons,
    std::vector<as_SkeletonAnimation*>& ani,
    std::vector<as_Material*>& materials,
    Vector3& sMax,
    Vector3& sMin);

