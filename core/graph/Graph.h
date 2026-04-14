#ifndef NETWORKANALYSISLAB_GRAPH_H
#define NETWORKANALYSISLAB_GRAPH_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Node.h"
#include "Edge.h"

class Graph {
private:
    int m_nextId = 0;
    std::unordered_map<int, Node> m_nodes;
    std::unordered_map<int, std::vector<int>> m_adjacentNodes;
    std::vector<Edge> m_edges;
public:
    int  addNode(const std::string& label, NodeType type);
    bool addEdge(int u, int v, float latency = 1.0f, float price = 1.0f,
                 float bandwidth = 1000.0f, float load = 0.0f, float reliability = 1.0f);
    const Edge* findEdge(int u, int v) const;
    bool removeEdge(int u, int v);
    bool removeNode(int id);

    bool hasNode(int id) const;
    const Node& getNode(int id)   const;
    const std::vector<int>&  neighbors(int id) const;
    const std::unordered_map<int, Node>& nodes() const;
    const std::vector<Edge>& edges() const;
    std::vector<int> getNodesByType(NodeType type) const;
};

#endif //NETWORKANALYSISLAB_GRAPH_H
