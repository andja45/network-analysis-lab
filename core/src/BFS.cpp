#include "BFS.h"
#include <queue>

BFSResult runBFS(const Graph& graph) {
    BFSResult result;

    for (const auto& [id, node] : graph.nodes()) {
        result.distance[id] = -1;
        result.nearestProvider[id] = -1;
        result.parent[id] = -1;
    }

    std::queue<int> q;

    for (int provider : graph.getNodesByType(NodeType::Provider)) {
        result.distance[provider] = 0;
        result.nearestProvider[provider] = provider;
        q.push(provider);
    }

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (int neighbor : graph.neighbors(current)) {
            if (result.distance[neighbor] == -1) { // unvisited
                result.distance[neighbor] = result.distance[current] + 1; 
                result.nearestProvider[neighbor] = result.nearestProvider[current];
                result.parent[neighbor] = current;
                q.push(neighbor);
            }
        }
    }

    return result;
}
