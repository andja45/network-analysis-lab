#ifndef NETWORKANALYSISLAB_REACHABILITYANALYZER_H
#define NETWORKANALYSISLAB_REACHABILITYANALYZER_H

#include <unordered_map>
#include "BFS.h"
#include "BridgeDetector.h"

enum class ConnectionCriticality { Critical, SemiCritical, Redundant };

struct BridgeImpact { // how many hosts become unreachable if this bridge fails
    Edge bridge;
    std::vector<int> disconnectedHosts;
};

struct ReachabilityResult {
    BFSResult bfsResult;
    BridgeResult bridgeResult;
    std::vector<BridgeImpact> bridgeImpacts;
    std::unordered_map<Edge, ConnectionCriticality> connectionCriticality;
    std::vector<int> unreachableHosts;
    std::vector<int> underservedHosts; // distance > maxHops
    int totalHosts = 0;
    int reachableHosts = 0;
};

ReachabilityResult analyze(const Graph& graph, int maxHops);

#endif
