cmake_minimum_required(VERSION 3.20)

project(GeoodeInstaller VERSION 1.0.0 LANGUAGES CXX RC)

add_executable(geoode
    src/app.cpp
    src/common.cpp
    src/console_ui.cpp
    src/installer.cpp
    src/main.cpp
    src/notifications.cpp
    src/registry.cpp
    src/geoode.rc
)

target_compile_features(geoode PRIVATE cxx_std_17)

target_compile_definitions(geoode PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0601
)

target_link_libraries(geoode PRIVATE
    advapi32
    shell32
    user32
)

if(MSVC)
    target_compile_options(geoode PRIVATE /W4 /permissive-)
else()
    target_compile_options(geoode PRIVATE -Wall -Wextra -Wpedantic)
endif()
