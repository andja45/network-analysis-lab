#include "Dijkstra.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>

static constexpr float INF = std::numeric_limits<float>::infinity();

struct Comparison {
    bool operator()(const std::pair<float, int>& a, const std::pair<float, int>& b) {
        return a.first > b.first;
    }
};

DijkstraResult runDijkstra(const Graph& graph, int src, int dst, Metric metric) {
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, Comparison> pq;
    std::unordered_map<int, int> parent; // (nodeId, parent in the path from src to nodeId)
    std::unordered_map<int, float> costs; // (nodeId, cost of path from src to nodeId)
    for (const auto& [id, _] : graph.nodes()) costs[id] = INF;
    std::unordered_set<int> visited;

    // init for src
    pq.push({0.0f, src});
    parent[src] = -1;
    costs[src] = 0.0f;

    DijkstraResult result;
    while (!pq.empty()) {
        auto [_, u] = pq.top(); pq.pop();

        if (visited.count(u)) continue;
        visited.insert(u);
        result.visitOrder.push_back(u);
        result.visited++;

        if (u == dst) break;

        for (int v : graph.neighbors(u)) {
            const Edge* e = graph.findEdge(u, v);
            float newCost = costs[u] + metric(*e);
            if (newCost < costs[v]) {
                costs[v] = newCost;
                parent[v] = u;
                pq.push({newCost, v});
                result.relaxations++;
            }
        }
    }

    // reconstructing the path, if found
    if (visited.count(dst)) {
        result.found = true;
        result.totalCost = costs[dst];
        for (int cur = dst; cur != -1; cur = parent[cur])
            result.path.push_back(cur);
        std::reverse(result.path.begin(), result.path.end());
    }

    return result;
}
