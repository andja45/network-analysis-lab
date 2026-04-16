#ifndef NETWORKANALYSISLAB_METRICS_H
#define NETWORKANALYSISLAB_METRICS_H

#include <functional>
#include <cmath>
#include <unordered_map>
#include <limits>
#include <utility>
#include "graph/Edge.h"
#include "graph/Graph.h"

using Metric = std::function<float(const Edge&)>;
using Heuristic = std::function<float(int, int)>;
using NodePositions = std::unordered_map<int, std::pair<float,float>>;

inline Metric costFastest() {
    return [](const Edge& e) { return e.latency; };
}

inline Metric costCheapest() {
    return [](const Edge& e) { return e.price; };
}

inline Metric costLeastLoaded() {
    // M/M/1 queueing model: wait = 1/(1-load)
    return [](const Edge& e) { return 1.0f / (1.0f - std::min(e.load, 0.99f)); };
}

inline Metric costMostReliable() {
    // max r1 x r2 x r3 is max log(r1) + log(r2) + log(r3) is min -log(r1) - log(r2) - log(r3)
    return [](const Edge& e) { return -std::log(std::max(e.reliability, 1e-6f)); };
}

inline Metric costBalanced() {
    // weights chosen so all three metrics contribute on similar scales
    return [](const Edge& e) {
        return 0.4f * e.latency + 0.4f * e.price + 0.2f * (-std::log(std::max(e.reliability, 1e-6f)));
    };
}

// h(n, goal) = r * d(n, goal), true cost >= h so heuristic is admissible
inline Heuristic buildAdmissibleHeuristic(const Graph& graph, Metric m, const NodePositions& pos) {
    float r = std::numeric_limits<float>::max();
    for (const auto& e : graph.edges()) {
        auto [x1,y1] = pos.at(e.from);
        auto [x2,y2] = pos.at(e.to);
        float d = std::hypot(x1-x2, y1-y2);
        if (d > 0) r = std::min(r, m(e) / d); // r = min(cost/length) over all edges — lowest cost-per-unit-length in the graph
    }
    if (!std::isfinite(r)) r = 0.0f;
    return [r, pos](int u, int v) {
        auto [ux,uy] = pos.at(u);
        auto [vx,vy] = pos.at(v);
        return r * std::hypot(ux-vx, uy-vy);
    };
}

#endif //NETWORKANALYSISLAB_METRICS_H
