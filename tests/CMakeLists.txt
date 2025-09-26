# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fdab60daffb7b6f6a746e7.zip
)

FetchContent_MakeAvailable(googletest)

# Тесты
add_executable(test_core test_core.cpp)
target_link_libraries(test_core PRIVATE
    tree-utility-core
    tree-utility-interfaces
    GTest::gtest_main
)

add_executable(test_filesystems test_filesystems.cpp)
target_link_libraries(test_filesystems PRIVATE
    tree-utility-filesystems
    GTest::gtest_main
)

add_executable(test_formatters test_formatters.cpp)
target_link_libraries(test_formatters PRIVATE
    tree-utility-formatters
    GTest::gtest_main
)

add_executable(test_visitors test_visitors.cpp)
target_link_libraries(test_visitors PRIVATE
    tree-utility-visitors
    GTest::gtest_main
)

# Запуск тестов
include(GoogleTest)
gtest_discover_tests(test_core)
gtest_discover_tests(test_filesystems)
gtest_discover_tests(test_formatters)
gtest_discover_tests(test_visitors)