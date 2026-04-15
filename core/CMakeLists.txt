add_library(core STATIC)

target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(core PRIVATE
    graph/Graph.cpp
    reachability/BFS.cpp
    reachability/BridgeDetector.cpp
    reachability/ReachabilityAnalyzer.cpp
    examples/GraphExamples.cpp
    routing/Dijkstra.cpp
    routing/AStar.cpp
    routing/RoutingAnalyzer.cpp
)

add_executable(core_demo main.cpp)
target_link_libraries(core_demo PRIVATE core)

target_compile_options(core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
)
