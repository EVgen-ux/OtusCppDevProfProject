cmake_minimum_required(VERSION 3.15)
project(TreeUtility VERSION 0.1 DESCRIPTION "Advanced directory tree utility")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/core)
add_subdirectory(src/builders)
add_subdirectory(src/cli)

add_executable(tree-utility src/main.cpp)


target_link_libraries(tree-utility 
    PRIVATE 
    CoreLib
    BuildersLib
    CliLib
)

find_package(CURL REQUIRED)

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
    nlohmann_json::nlohmann_json
)

install(TARGETS tree-utility DESTINATION bin)

set(CPACK_PACKAGE_NAME "tree-utility")
set(CPACK_PACKAGE_VENDOR "TreeUtility Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced directory tree utility")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TreeUtility Maintainer <maintainer@example.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4, libc6, libstdc++6")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_GENERATOR "DEB")

include(CPack)