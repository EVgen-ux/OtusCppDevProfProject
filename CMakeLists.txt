cmake_minimum_required(VERSION 3.14)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Используем FetchContent для nlohmann_json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Поиск библиотеки CURL
find_package(CURL REQUIRED)
if(NOT CURL_FOUND)
    # Попробуем найти curl через pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CURL libcurl REQUIRED)
    
    if(NOT CURL_FOUND)
        message(FATAL_ERROR "CURL library not found. Install libcurl-dev package: sudo apt-get install libcurl4-openssl-dev")
    endif()
endif()

message(STATUS "CURL found: ${CURL_VERSION}")
message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")

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
    src/GitHubTreeBuilder.cpp
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
    include/GitHubTreeBuilder.h
)

add_executable(tree-utility ${SOURCES} ${HEADERS})

target_include_directories(tree-utility 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CURL_INCLUDE_DIRS}
)

# Подключаем nlohmann_jso
target_link_libraries(tree-utility 
    PRIVATE 
        nlohmann_json
        ${CURL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(tree-utility PRIVATE pthread)
endif()