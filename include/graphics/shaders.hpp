#pragma once

#include "graphics/shader.hpp"


namespace shaders {

using meshes = shader<"proj_mat", "view_mat", "model_mat", "color_merge", "uniform_color">;
using mesh_lines = shader<"proj_mat", "view_mat", "model_mat", "color_merge", "uniform_color">;
using mesh_points = shader<"proj_mat", "view_mat", "model_mat", "color_merge", "uniform_color", "point_size">;
using points = shader<"proj_mat", "view_mat", "model_mat", "uniform_color", "point_size">;

} // namespace shaders
