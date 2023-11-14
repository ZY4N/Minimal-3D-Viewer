cmake_minimum_required(VERSION 3.27)
project(3d_viewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -pedantic -Wall -Werror ")

add_executable(3d_viewer main.cpp
        source/graphics/camera.cpp
        source/graphics/renderers/mesh_renderer.cpp
        source/graphics/renderers/mesh_line_renderer.cpp
        source/graphics/renderers/mesh_point_renderer.cpp
        source/graphics/flying_camera.cpp
        include/geometry/mesh.hpp
        source/geometry/mesh.ipp
        include/geometry/mesh_loader.hpp
        source/geometry/mesh_loader.ipp
        include/geometry/vertex_component.hpp
        include/graphics/renderable_attributes/color_attribute.hpp
        include/graphics/camera.hpp
        include/graphics/flying_camera.hpp
        include/graphics/renderers/mesh_line_renderer.hpp
        include/graphics/renderers/mesh_renderer.hpp
        include/graphics/renderable_attribute.hpp
        include/graphics/renderers/mesh_point_renderer.hpp
        include/graphics/renderables/mesh_instance.hpp
        include/graphics/renderers/renderer.hpp
        include/graphics/shader.hpp
        source/graphics/shader.ipp
        include/graphics/renderable_attributes/texture_attribute.hpp
        include/util/arx.hpp
        include/util/string_literal.hpp
        include/util/for_each.hpp
        include/util/function.hpp
        include/util/image.hpp
        include/util/logger.hpp
        include/util/pack.hpp
        include/util/uix.hpp
        include/util/string_indexer.hpp
        include/util/rgb_color.hpp
        include/util/rgba_color.hpp
        include/graphics/renderables/point_cloud_instance.hpp
        source/graphics/renderers/point_renderer.cpp
        include/graphics/dynamic_renderable_attribute.hpp
        include/graphics/renderers/point_cloud_renderer.hpp
        include/graphics/shaders.hpp
        include/geometry/aabb.hpp
        include/graphics/renderable_attributes/point_size_attribute.hpp
        include/geometry/point_cloud_loader.hpp
        source/geometry/point_cloud_loader.ipp
        include/geometry/point_cloud.hpp
        source/geometry/point_cloud.ipp
        include/util/extra_arx_parsers.hpp
        include/geometry/material.hpp
        include/graphics/to_gl_type.hpp
)

target_include_directories(3d_viewer PRIVATE include)
target_include_directories(3d_viewer PRIVATE source) # for ipp headers
target_include_directories(3d_viewer PRIVATE libraries/include/glm)
target_include_directories(3d_viewer PRIVATE libraries/include/stb)


find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SFML REQUIRED COMPONENTS graphics system)
include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(3d_viewer sfml-graphics sfml-system sfml-window ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
