cmake_minimum_required(VERSION 3.10)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

set(SOURCES
    src/main.cpp
    src/BaseTreeBuilder.cpp
    src/TreeBuilder.cpp
    src/FileSystem.cpp
    src/DepthViewTreeBuilder.cpp
    src/ITreeBuilder.cpp
)

set(HEADERS
    include/constants.h
    include/ITreeBuilder.h
    include/BaseTreeBuilder.h
    include/TreeBuilder.h
    include/FileSystem.h
    include/DepthViewTreeBuilder.h
)

add_executable(tree-utility ${SOURCES} ${HEADERS})

target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(WIN32)
    target_link_libraries(tree-utility ole32 oleaut32 uuid)
endif()