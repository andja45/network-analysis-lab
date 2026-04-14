#ifndef NETWORKANALYSISLAB_DIJKSTRA_H
#define NETWORKANALYSISLAB_DIJKSTRA_H

#include <vector>
#include "Metrics.h"
#include "graph/Graph.h"

struct DijkstraResult {
    std::vector<int> path;
    std::vector<int> visitOrder;
    float totalCost = 0.0f; // total cost of the path found
    int relaxations = 0; // how many times a cost value was improved
    int visited = 0;
    bool found = false;
};

DijkstraResult runDijkstra(const Graph& graph, int src, int dst, Metric metric);

#endif //NETWORKANALYSISLAB_DIJKSTRA_H
