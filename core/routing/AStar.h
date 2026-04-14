#ifndef NETWORKANALYSISLAB_ASTAR_H
#define NETWORKANALYSISLAB_ASTAR_H

#include <vector>
#include "Metrics.h"
#include "graph/Graph.h"

struct AStarResult {
    std::vector<int> path;
    std::vector<int> visitOrder;
    float totalCost = 0.0f; // total cost of the path found
    int relaxations = 0; // how many times a cost value was improved
    int visited = 0;
    bool found = false;
};

AStarResult runAStar(const Graph& graph, int src, int dst, Metric metric, Heuristic h);

#endif //NETWORKANALYSISLAB_ASTAR_H
