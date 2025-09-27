add_library(BuildersLib STATIC
    DepthViewTreeBuilder.cpp
    FilteredTreeBuilder.cpp
    MultiThreadedTreeBuilder.cpp
    JSONTreeBuilder.cpp
    GitHubTreeBuilder.cpp
)



target_include_directories(BuildersLib
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)


target_include_directories(BuildersLib
    PUBLIC
    ${nlohmann_json_INCLUDE_DIRS}  
)

target_link_libraries(BuildersLib 
    PUBLIC
    CoreLib
    nlohmann_json::nlohmann_json
)
