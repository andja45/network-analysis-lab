#include "AppState.h"
#include <unordered_set>
#include <algorithm>

void AppState::runAnalysis() {
    result = analyze(graph, maxHops);

    std::unordered_set<Edge> bridges;
    for (const BridgeImpact& impact : result.bridgeImpacts)
        bridges.insert(impact.bridge);

    edgeConnection.clear();
    for (const Edge& e : graph.edges()) {
        if (!bridges.count(e)) {
            edgeConnection[e] = EdgeConnection::Redundant;
            continue;
        }

        auto it = std::find_if(result.bridgeImpacts.begin(), result.bridgeImpacts.end(),
            [&](const BridgeImpact& b) { return b.bridge.from == e.from && b.bridge.to == e.to; });

        edgeConnection[e] = it->disconnectedHosts.empty()
            ? EdgeConnection::SemiCritical
            : EdgeConnection::Critical;
    }
}
