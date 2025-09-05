cmake_minimum_required(VERSION 3.10)
project(TreeUtility)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка компилятора
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Исходные файлы
set(SOURCES
    src/main.cpp
    src/TreeBuilder.cpp
    src/FileSystem.cpp
)

# Заголовочные файлы
set(HEADERS
    include/constants.h
    src/TreeBuilder.h
    src/FileSystem.h
)

# Создание исполняемого файла
add_executable(tree-utility ${SOURCES} ${HEADERS})

# Включение директорий
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(tree-utility PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Для Windows нужно линковать с определенными библиотеками
if(WIN32)
    target_link_libraries(tree-utility ole32 oleaut32 uuid)
endif()