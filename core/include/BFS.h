#ifndef NETWORKREACHABILITYANALYSIS_BFS_H
#define NETWORKREACHABILITYANALYSIS_BFS_H

#include <unordered_map>
#include "Graph.h"

struct BFSResult {
    std::unordered_map<int, int> distance; // hops to nearest provider (node id, num of hops)
    std::unordered_map<int, int> nearestProvider; // which provider is nearest (node id, provider id)
    std::unordered_map<int, int> parent; // for path reconstruction (node id, parent node id in BFS tree)
};

BFSResult runBFS(const Graph& graph);

#endif
