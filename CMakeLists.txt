cmake_minimum_required(VERSION 3.10)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/BaseTreeBuilder.cpp
    src/TreeBuilder.cpp
    src/FileSystem.cpp
    src/DepthViewTreeBuilder.cpp
    src/ITreeBuilder.cpp
    src/FilteredTreeBuilder.cpp
    src/CommandLineParser.cpp
    src/HelpPrinter.cpp
    src/StatisticsPrinter.cpp
)

set(HEADERS
    include/constants.h
    include/ITreeBuilder.h
    include/BaseTreeBuilder.h
    include/TreeBuilder.h
    include/FileSystem.h
    include/DepthViewTreeBuilder.h
    include/FilteredTreeBuilder.h
    include/CommandLineParser.h
    include/HelpPrinter.h
    include/StatisticsPrinter.h
)

add_executable(tree-utility ${SOURCES} ${HEADERS})

target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/src)
