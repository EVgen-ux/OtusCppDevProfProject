cmake_minimum_required(VERSION 3.10)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для поддержки цветов в Windows
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0600)
    add_compile_definitions(NOMINMAX)
endif()

set(SOURCES
    src/main.cpp
    src/BaseTreeBuilder.cpp
    src/TreeBuilder.cpp
    src/FileSystem.cpp
    src/DepthViewTreeBuilder.cpp
    src/ITreeBuilder.cpp
    src/FilteredTreeBuilder.cpp
    src/ColorManager.cpp
)

set(HEADERS
    include/constants.h
    include/ITreeBuilder.h
    include/BaseTreeBuilder.h
    include/TreeBuilder.h
    include/FileSystem.h
    include/DepthViewTreeBuilder.h
    include/FilteredTreeBuilder.h
    include/ColorManager.h 
)

add_executable(tree-utility ${SOURCES} ${HEADERS})

target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/src)
