#include "AStar.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <limits>

static constexpr float INF = std::numeric_limits<float>::infinity();

AStarResult runAStar(const Graph& graph, int src, int dst, Metric metric, Heuristic h) {
    std::unordered_set<int> open, closed;
    std::unordered_map<int, float> g;
    for (const auto& [id, _] : graph.nodes()) g[id] = INF;
    std::unordered_map<int, int> parent;

    // init for src
    open.insert(src);
    g[src] = 0.0f;
    parent[src] = -1;

    AStarResult result;
    while (!open.empty()) {
        // choosing the best node from the open set
        int u = -1;
        float bestF = INF;
        for (int node : open) {
            float f = g[node] + h(node, dst);
            if (f < bestF) { bestF = f; u = node; }
        }

        result.visitOrder.push_back(u);
        result.visited++;

        // reconstructing the path, if found
        if (u == dst) {
            result.found = true;
            result.totalCost = g[dst];
            for (int cur = dst; cur != -1; cur = parent[cur])
                result.path.push_back(cur);
            std::reverse(result.path.begin(), result.path.end());
            return result;
        }

        for (int v : graph.neighbors(u)) {
            const Edge* e = graph.findEdge(u, v);
            float newG = g[u] + metric(*e);
            if (newG < g[v]) {
                g[v] = newG;
                parent[v] = u;
                open.insert(v); // add or keep v in open with updated cost
                result.relaxations++;
                if (closed.count(v)) closed.erase(v); // if it was in closed, we need to reconsider it
            }
        }

        // we have considered all neighbors of u, we can move it to closed
        open.erase(u);
        closed.insert(u);
    }

    return result;
}
