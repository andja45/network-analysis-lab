#ifndef NETWORKREACHABILITYANALYSIS_GRAPH_H
#define NETWORKREACHABILITYANALYSIS_GRAPH_H

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
    bool addEdge(int u, int v);
    bool removeEdge(int u, int v);
    bool removeNode(int id);

    bool hasNode(int id) const;
    const Node& getNode(int id)   const;
    const std::vector<int>&  neighbors(int id) const;
    const std::unordered_map<int, Node>& nodes() const;
    const std::vector<Edge>& edges() const; // all edges, no duplicates
    std::vector<int> getNodesByType(NodeType type) const;
};

#endif //NETWORKREACHABILITYANALYSIS_GRAPH_H
