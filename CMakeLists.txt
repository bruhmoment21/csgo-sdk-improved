cmake_minimum_required(VERSION 3.23)
project(csgo-sdk)

# DEBUG or RELEASE
set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_FLAGS "-march=native --no-gnu-unique")
set(CMAKE_C_FLAGS_DEBUG "-g3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -s -fvisibility=hidden -DNDEBUG")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# This is needed for funchook.
add_compile_definitions(DISASM_DISTORM)

file(GLOB_RECURSE SDK_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/src/*.cpp")
file(GLOB_RECURSE FUNCHOOK_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies/funchook/*.c")
file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies/imgui/*.cpp")

# Remove Windows specific source files.
list(REMOVE_ITEM SDK_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/src/hooks/backend/win32/directx9_hook.cpp")
list(REMOVE_ITEM SDK_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/src/hooks/backend/win32/wndproc_hook.cpp")

list(REMOVE_ITEM FUNCHOOK_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies/funchook/src/funchook_windows.c")

list(REMOVE_ITEM IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies/imgui/imgui_impl_win32.cpp")
list(REMOVE_ITEM IMGUI_SOURCES "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies/imgui/imgui_impl_dx9.cpp")

# Enables warnings for our source files.
set_property(SOURCE ${SDK_SOURCES} PROPERTY COMPILE_FLAGS "-Wall")

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Freetype REQUIRED)

add_library(${CMAKE_PROJECT_NAME} SHARED ${SDK_SOURCES} ${FUNCHOOK_SOURCES} ${IMGUI_SOURCES})

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE LINKER:--no-undefined)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} ${Vulkan_LIBRARIES} ${FREETYPE_LIBRARIES})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/csgo-sdk-improved/dependencies"

    ${FREETYPE_INCLUDE_DIRS}
)
