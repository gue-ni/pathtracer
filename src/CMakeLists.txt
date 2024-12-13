cmake_minimum_required(VERSION 3.18)
project(pathtracer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b # v1.0.0
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
  message("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_library(pathtracer
  "pt.cpp"
  "camera.cpp"
  "geometry.cpp"
  "ray.cpp"
  "renderer.cpp"
  "scene.cpp"
  "material.cpp"
  "aabb.cpp"
  "bvh.cpp"
  "tiny_obj_loader.cpp"
  "image.cpp"
  "medium.cpp"
)

target_link_libraries(pathtracer PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(pathtracer PUBLIC "${glm_SOURCE_DIR}" "${json_SOURCE_DIR}" ".")

add_executable(pt "main.cpp")
target_link_libraries(pt PRIVATE pathtracer)