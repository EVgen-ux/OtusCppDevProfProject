add_library(CliLib STATIC
    BuilderFactory.cpp
    CommandLineParser.cpp
    OutputManager.cpp
)

target_include_directories(CliLib
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../builders
)


target_link_libraries(CliLib 
    PUBLIC  
    CoreLib
    BuildersLib
)