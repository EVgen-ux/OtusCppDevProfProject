cmake_minimum_required(VERSION 3.14)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Для поддержки цветов в Windows
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0600)
    add_compile_definitions(NOMINMAX)
endif()

# Используем FetchContent для nlohmann_json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

set(SOURCES
    src/main.cpp
    src/BaseTreeBuilder.cpp
    src/TreeBuilder.cpp
    src/FileSystem.cpp
    src/DepthViewTreeBuilder.cpp
    src/ITreeBuilder.cpp
    src/FilteredTreeBuilder.cpp
    src/ColorManager.cpp
    src/JSONTreeBuilder.cpp
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
    include/JSONTreeBuilder.h
)

add_executable(tree-utility ${SOURCES} ${HEADERS})

target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Подключаем nlohmann_json
target_link_libraries(tree-utility PRIVATE nlohmann_json)