cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
            CACHE PATH "VitaSDK toolchain file")
    else()
        message(FATAL_ERROR "Set VITASDK to the location of your VitaSDK installation")
    endif()
endif()

project(vita_minesweeper LANGUAGES C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)
find_package(SDL2 CONFIG REQUIRED)

set(VITA_APP_NAME "Minesweeper")
set(VITA_TITLEID "VMIN00001")
set(VITA_VERSION "01.00")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/board.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2main
    SDL2::SDL2-static
)

# Excluded from Vita package builds; this entry gives clangd an accurate
# compile command for the host-side board test.
add_library(board_test_source OBJECT EXCLUDE_FROM_ALL tests/board_test.cpp)
target_include_directories(board_test_source PRIVATE src)
target_compile_features(board_test_source PRIVATE cxx_std_17)
target_compile_options(board_test_source PRIVATE -UNDEBUG)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE sce_sys/icon0.png sce_sys/icon0.png
    FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
