cmake_minimum_required(VERSION 3.15)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(cpp-gl-gui VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Bring in cpp-core if not already present
if(NOT TARGET cpp-core)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../cpp-core
                     ${CMAKE_CURRENT_BINARY_DIR}/cpp-core-build)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

add_library(cpp-gl-gui STATIC
    src/Shader.cpp
    src/Texture.cpp
    src/VertexBuffer.cpp
    src/VertexArray.cpp
    src/IndexBuffer.cpp
    src/Renderer.cpp
    src/gui_core.cpp
    src/image_utils.cpp
)

# PUBLIC include dirs: consumers get gl-gui headers + GLEW + Stb automatically
target_include_directories(cpp-gl-gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLEW_INCLUDE_DIRS}
    ${Stb_INCLUDE_DIR}
)

# All GL/GUI deps are PUBLIC so consumers (e.g. NNUI) don't need to repeat them
target_link_libraries(cpp-gl-gui PUBLIC
    cpp-core
    fmt::fmt
    FFTW3::fftw3
    OpenGL::GL
    ${GLEW_LIBRARIES}
    glm::glm
    glfw
    imgui::imgui
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
