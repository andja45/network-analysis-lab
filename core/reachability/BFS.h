#ifndef NETWORKREACHABILITYANALYSIS_BFS_H
#define NETWORKREACHABILITYANALYSIS_BFS_H

#include <unordered_map>
#include "graph/Graph.h"

struct BFSResult {
    std::unordered_map<int, int> distance;       // hops to the nearest provider
    std::unordered_map<int, int> nearestProvider; // which provider is nearest
    std::unordered_map<int, int> parent;          // parent in BFS tree
    std::vector<std::vector<int>> bfsLevels;      // nodes grouped by hops from the nearest provider
};

BFSResult runBFS(const Graph& graph);

#endif
