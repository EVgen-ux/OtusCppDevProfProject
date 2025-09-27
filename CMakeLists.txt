cmake_minimum_required(VERSION 3.15)
project(TreeUtility VERSION 2.0.0 DESCRIPTION "Advanced directory tree utility")

# Настройка C++ стандарта
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение поддиректорий
add_subdirectory(src/core)
add_subdirectory(src/builders)
add_subdirectory(src/cli)

# Основной исполняемый файл
add_executable(tree-utility src/main.cpp)

# Связывание с внутренними библиотеками
target_link_libraries(tree-utility 
    PRIVATE 
    CoreLib
    BuildersLib
    CliLib
)

# Подключение внешних зависимостей
find_package(CURL REQUIRED)

# Решение для nlohmann_json через FetchContent
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(tree-utility 
    PRIVATE 
    CoreLib
    BuildersLib
    CliLib
    CURL::libcurl
)

# Установка
install(TARGETS tree-utility DESTINATION bin)