#include "Graph.h"
#include <algorithm>

int Graph::addNode(const std::string& label, NodeType type) {
    int id = m_nextId++;
    m_nodes[id] = Node{id, label, type};
    m_adjacentNodes[id] = {};
    return id;
}

bool Graph::addEdge(int u, int v, float latency, float price,
                    float bandwidth, float load, float reliability) {
    if (!hasNode(u) || !hasNode(v) || u == v) return false;

    auto& uNeighbors = m_adjacentNodes[u];
    auto& vNeighbors = m_adjacentNodes[v];

    if (std::find(uNeighbors.begin(), uNeighbors.end(), v) != uNeighbors.end()) return false;

    uNeighbors.push_back(v);
    vNeighbors.push_back(u);
    m_edges.push_back({std::min(u, v), std::max(u, v), latency, price, bandwidth, load, reliability});

    return true;
}

const Edge* Graph::findEdge(int u, int v) const {
    int a = std::min(u, v), b = std::max(u, v);
    for (const auto& e : m_edges)
        if (e.from == a && e.to == b) return &e;
    return nullptr;
}

bool Graph::removeEdge(int u, int v) {
    if (!hasNode(u) || !hasNode(v)) return false;

    auto& uNeighbors = m_adjacentNodes[u];
    auto& vNeighbors = m_adjacentNodes[v];

    auto it = std::find(uNeighbors.begin(), uNeighbors.end(), v);
    if (it == uNeighbors.end()) return false;

    uNeighbors.erase(it);
    vNeighbors.erase(std::find(vNeighbors.begin(), vNeighbors.end(), u));

    int from = std::min(u, v);
    int to  = std::max(u, v);

    for (auto it = m_edges.begin(); it != m_edges.end(); ++it) {
        if (it->from == from && it->to == to) {
            m_edges.erase(it);
            break;
        }
    }

    return true;
}

bool Graph::removeNode(int id) {
    if (!hasNode(id)) return false;

    std::vector<int> neighbors = m_adjacentNodes[id];
    for (int neighbor : neighbors) removeEdge(id, neighbor);

    m_nodes.erase(id);
    m_adjacentNodes.erase(id);
    return true;
}

bool Graph::hasNode(int id) const {
    return m_nodes.find(id) != m_nodes.end();
}

const Node& Graph::getNode(int id) const {
    return m_nodes.at(id);
}

const std::vector<int>& Graph::neighbors(int id) const {
    return m_adjacentNodes.at(id);
}

const std::unordered_map<int, Node>& Graph::nodes() const {
    return m_nodes;
}

const std::vector<Edge>& Graph::edges() const {
    return m_edges;
}

std::vector<int> Graph::getNodesByType(NodeType type) const {
    std::vector<int> result;
    for (const auto& [id, node] : m_nodes) {
        if (node.type == type) result.push_back(id);
    }
    return result;
}