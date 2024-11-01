cmake_minimum_required(VERSION 3.18)
project(pathtracer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

if(MSVC)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 33b0eb9fa336ffd8551024b1d2690e418014553b # v1.0.0
)
FetchContent_MakeAvailable(glm)

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
  message("OPENMP FOUND")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(pt
  "src/main.cpp"
  "src/camera.cpp"
  "src/geometry.cpp"
  "src/ray.cpp"
  "src/renderer.cpp"
  "src/scene.cpp"
  "src/material.cpp"
  "src/kdtree.cpp"
)

target_include_directories(pt PUBLIC "${glm_SOURCE_DIR}")