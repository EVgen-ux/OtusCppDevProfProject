cmake_minimum_required(VERSION 3.15)
project(TreeUtility VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка выходных директорий
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Загрузка nlohmann/json через FetchContent
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# Поиск CURL с проверкой
find_package(PkgConfig REQUIRED)
find_package(CURL)  # Без REQUIRED, чтобы проверить наличие

# Проверяем, найден ли CURL
if(NOT CURL_FOUND)
    message(WARNING "CURL not found. GitHub support will be disabled.")
    set(ENABLE_GITHUB_SUPPORT OFF)
else()
    set(ENABLE_GITHUB_SUPPORT ON)
    message(STATUS "CURL found: ${CURL_VERSION}")
endif()

# Основной исполняемый файл
add_executable(tree-utility)

# Базовые исходные файлы
target_sources(tree-utility PRIVATE
    src/main.cpp
    src/TreeBuilder.cpp
    src/FilteredTreeBuilder.cpp
    src/DepthViewTreeBuilder.cpp
    src/JSONTreeBuilder.cpp
    src/FileSystem.cpp
    src/ColorManager.cpp
)

# Условное добавление GitHubTreeBuilder
if(ENABLE_GITHUB_SUPPORT)
    target_sources(tree-utility PRIVATE src/GitHubTreeBuilder.cpp)
    message(STATUS "GitHub support enabled")
else()
    message(STATUS "GitHub support disabled - CURL not found")
endif()

# Настройки include
target_include_directories(tree-utility PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Условная компоновка с CURL
if(ENABLE_GITHUB_SUPPORT)
    target_link_libraries(tree-utility PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
    )
else()
    target_link_libraries(tree-utility PRIVATE
        nlohmann_json::nlohmann_json
    )
endif()

# Определение для условной компиляции
if(ENABLE_GITHUB_SUPPORT)
    target_compile_definitions(tree-utility PRIVATE ENABLE_GITHUB_SUPPORT=1)
else()
    target_compile_definitions(tree-utility PRIVATE ENABLE_GITHUB_SUPPORT=0)
endif()

# Установка
install(TARGETS tree-utility
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Дополнительные настройки для Unix-систем
if(UNIX AND NOT APPLE)
    # Настройки для Linux
    find_package(Threads REQUIRED)
    target_link_libraries(tree-utility PRIVATE Threads::Threads)
endif()

# Политики CMake для избежания предупреждений
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

# Показываем итоговую конфигурацию
message(STATUS "Tree Utility configuration:")
message(STATUS "  - C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - GitHub support: ${ENABLE_GITHUB_SUPPORT}")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - Install prefix: ${CMAKE_INSTALL_PREFIX}")