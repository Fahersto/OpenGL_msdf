cmake_minimum_required (VERSION 3.8)

project ("OpenGL_msdf")

include(../vcpkg/scripts/buildsystems/vcpkg.cmake)


add_subdirectory(msdf-atlas-gen)

file(GLOB_RECURSE EXAMPLE_SOURCES LIST_DIRECTORIES true src/*.cpp)



add_executable (opengl_msdfatlas ${EXAMPLE_SOURCES})


find_package(glfw3 REQUIRED)
target_link_libraries(opengl_msdfatlas PRIVATE glfw)

find_package(glad REQUIRED)
target_link_libraries(opengl_msdfatlas PRIVATE glad::glad)

find_package(glm REQUIRED)
target_link_libraries(opengl_msdfatlas PRIVATE glm::glm)

target_link_libraries(opengl_msdfatlas PRIVATE msdf-atlas-gen)



target_compile_features(opengl_msdfatlas PRIVATE cxx_std_17)


# vcpkg install freetype:x64-windows