#ifndef NETWORKREACHABILITYANALYSIS_ANALYZER_H
#define NETWORKREACHABILITYANALYSIS_ANALYZER_H

#include <unordered_map>
#include "BFS.h"
#include "BridgeDetector.h"

enum class EdgeCriticality { Critical, SemiCritical, Redundant };

struct BridgeImpact {
    Edge bridge;
    std::vector<int> disconnectedHosts;
};

struct AnalysisResult {
    BFSResult bfs;
    BridgeResult bridgeResult;
    std::vector<BridgeImpact> bridgeImpacts;
    std::unordered_map<Edge, EdgeCriticality> edgeCriticality;
    std::vector<int> unreachableHosts;
    std::vector<int> underservedHosts; // distance > maxHops
    int totalHosts = 0;
    int reachableHosts = 0;
};

AnalysisResult analyze(const Graph& graph, int maxHops);

#endif
