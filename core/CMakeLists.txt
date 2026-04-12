add_library(core STATIC)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(core PRIVATE
    src/Graph.cpp
    src/BFS.cpp
    src/BridgeDetector.cpp
    src/Analyzer.cpp
    src/GraphExamples.cpp
)

add_executable(core_demo src/main.cpp)

target_link_libraries(core_demo PRIVATE core)

target_compile_options(core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
)