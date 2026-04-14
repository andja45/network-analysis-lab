#include "ReachabilityAnalyzer.h"
#include <unordered_set>

ReachabilityResult analyze(const Graph& graph, int maxHops) {
    ReachabilityResult result;

    result.bfsResult = runBFS(graph);

    for (int hostId : graph.getNodesByType(NodeType::Host)) {
        result.totalHosts++;
        if (result.bfsResult.distance[hostId] == -1) {
            result.unreachableHosts.push_back(hostId);
        } else {
            result.reachableHosts++;
            if (maxHops != -1 && result.bfsResult.distance[hostId] > maxHops)
                result.underservedHosts.push_back(hostId);
        }
    }

    result.bridgeResult = findBridges(graph);

    // for each bridge, simulate its removal and record which hosts lose connectivity
    for (const Edge& bridge : result.bridgeResult.bridges) {
        BridgeImpact impact;
        impact.bridge = bridge;

        Graph copy = graph;
        copy.removeEdge(bridge.from, bridge.to);
        BFSResult impactBFS = runBFS(copy);

        for (int hostId : graph.getNodesByType(NodeType::Host)) {
            bool wasReachable   = result.bfsResult.distance[hostId] != -1;
            bool nowUnreachable = impactBFS.distance[hostId] == -1;
            if (wasReachable && nowUnreachable)
                impact.disconnectedHosts.push_back(hostId);
        }

        result.bridgeImpacts.push_back(impact);
    }

    std::unordered_set<Edge> criticalBridges;
    for (const BridgeImpact& b : result.bridgeImpacts)
        if (!b.disconnectedHosts.empty())
            criticalBridges.insert(b.bridge);

    for (const Edge& e : graph.edges()) {
        if (criticalBridges.count(e)) {
            result.connectionCriticality[e] = ConnectionCriticality::Critical;
            continue;
        }

        // simulate removing e from the original graph
        Graph modified = graph;
        modified.removeEdge(e.from, e.to);

        bool newCriticalFound = false;
        for (const Edge& bridge : findBridges(modified).bridges) {
            if (criticalBridges.count(bridge)) continue; // removing an edge keeps or increases the number of critical bridges

            Graph modified2 = modified;
            modified2.removeEdge(bridge.from, bridge.to);
            BFSResult impactBFS = runBFS(modified2);

            for (int hostId : graph.getNodesByType(NodeType::Host)) {
                bool wasReachable   = result.bfsResult.distance[hostId] != -1;
                bool nowUnreachable = impactBFS.distance[hostId] == -1;
                if (wasReachable && nowUnreachable) {
                    newCriticalFound = true;
                    break;
                }
            }
            if (newCriticalFound) break; // if it increases the number of critical bridges by even 1, we have a semi-critical connection
        }

        result.connectionCriticality[e] = newCriticalFound
            ? ConnectionCriticality::SemiCritical
            : ConnectionCriticality::Redundant;
    }

    return result;
}
