cmake_minimum_required(VERSION 3.4...3.18)
project(geometry LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 17)

add_subdirectory(pybind11)

find_package(Python COMPONENTS Interpreter Development REQUIRED)

include_directories(metacity/geometry)
link_directories(metacity/geometry)

#add_compile_options(-Wall -pedantic -g -fsanitize=address -fno-omit-frame-pointer -shared-libasan)
add_compile_options(-Wall -pedantic -O3)
#add_link_options(-fsanitize=address -shared-libasan -pthread)
add_link_options(-pthread)


pybind11_add_module(${PROJECT_NAME} metacity/geometry/geometry.cpp 
                                    metacity/geometry/progress.hpp 
                                    metacity/geometry/bbox.hpp 
                                    metacity/geometry/bbox.cpp 
                                    metacity/geometry/simplify.hpp 
                                    metacity/geometry/simplify.cpp 
                                    metacity/geometry/mapping.hpp 
                                    metacity/geometry/mapping.cpp 
                                    metacity/geometry/graham.hpp 
                                    metacity/geometry/graham.cpp 
                                    metacity/geometry/bvh.hpp 
                                    metacity/geometry/bvh.cpp 
                                    metacity/geometry/model.hpp 
                                    metacity/geometry/model.cpp 
                                    metacity/geometry/layer.hpp 
                                    metacity/geometry/layer.cpp 
                                    metacity/geometry/attribute.hpp 
                                    metacity/geometry/attribute.cpp 
                                    metacity/geometry/types.hpp
                                    metacity/geometry/grid.hpp
                                    metacity/geometry/grid.cpp
                                    metacity/geometry/triangulation.hpp
                                    metacity/geometry/triangulation.cpp)


