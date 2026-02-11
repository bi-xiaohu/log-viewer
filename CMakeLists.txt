cmake_minimum_required(VERSION 3.20)
project(LogViewer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /utf-8)
    # 静态链接运行时库
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # MSYS2/MinGW静态链接
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# 第三方库路径
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# ====== GLFW ======
add_subdirectory(${THIRD_PARTY_DIR}/glfw)

# ====== ImGui ======
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${THIRD_PARTY_DIR}/glfw/include
)

# ====== SQLite3 ======
add_library(sqlite3 STATIC ${THIRD_PARTY_DIR}/sqlite/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${THIRD_PARTY_DIR}/sqlite)

# ====== nlohmann/json ======
set(JSON_INCLUDE_DIR ${THIRD_PARTY_DIR}/json/include)

# ====== 主程序 ======
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/core/*.cpp
    src/gui/*.cpp
    src/utils/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${JSON_INCLUDE_DIR}
    ${THIRD_PARTY_DIR}/sqlite
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    glfw
    sqlite3
    opengl32
)

# 静态链接所有依赖
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Release优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2 /GL)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()
