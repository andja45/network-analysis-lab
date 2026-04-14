#include "BridgeDetector.h"
#include <unordered_map>
#include <algorithm>

// Tarjan's algorithm for finding bridges in an undirected graph
static void dfs(int u, int parent, const Graph& graph, std::unordered_map<int, int>& dfsNum,
                std::unordered_map<int, int>& lowlink, int& num,
                std::vector<Edge>& bridges, std::vector<Edge>& dfsOrder) {

    dfsNum[u] = lowlink[u] = ++num;

    for (int v : graph.neighbors(u)) {
        if (dfsNum[v] == 0) { // unvisited
            dfsOrder.push_back({u, v});
            dfs(v, u, graph, dfsNum, lowlink, num, bridges, dfsOrder);

            lowlink[u] = std::min(lowlink[u], lowlink[v]);

            if (lowlink[v] > dfsNum[u])
                bridges.push_back({std::min(u, v), std::max(u, v)});
        } else if (v != parent) {
            lowlink[u] = std::min(lowlink[u], dfsNum[v]);
        }
    }
}

BridgeResult findBridges(const Graph& graph) {
    std::unordered_map<int, int> dfsNum;
    std::unordered_map<int, int> lowLink;
    BridgeResult result;
    int num = 0;

    for (const auto& [id, node] : graph.nodes())
        dfsNum[id] = 0;
    
    for (const auto& [id, node] : graph.nodes())
        if (dfsNum[id] == 0)
            dfs(id, -1, graph, dfsNum, lowLink, num, result.bridges, result.dfsOrder);

    return result;
}
