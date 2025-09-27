add_library(CoreLib STATIC
    TreeBuilder.cpp
    FileSystem.cpp
    ColorManager.cpp
)

target_include_directories(CoreLib PUBLIC .)
