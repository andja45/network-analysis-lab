#include <iostream>
#include "Graph.h"

void printAdjacency(const Graph& graph) {
    for (const auto& [id, node] : graph.nodes()) {
        std::cout << node.label << " -> ";
        for (int neighbor : graph.neighbors(id)) {
            std::cout << graph.getNode(neighbor).label << "  ";
        }
        std::cout << "\n";
    }
}

void printEdges(const Graph& graph) {
    for (const Edge& edge : graph.edges()) {
        std::cout << "{" << edge.from << ", " << edge.to << "}  ";
    }
    std::cout << "\n";
}

int main() {
    Graph graph;

    int provider = graph.addNode("Provider A", NodeType::Provider);
    int router1  = graph.addNode("Router 1",   NodeType::Router);
    int router2  = graph.addNode("Router 2",   NodeType::Router);
    int host1    = graph.addNode("Host 1",     NodeType::Host);
    int host2    = graph.addNode("Host 2",     NodeType::Host);

    graph.addEdge(provider, router1);
    graph.addEdge(router1,  router2);
    graph.addEdge(router2,  host1);
    graph.addEdge(router2,  host2);

    std::cout << "Adjacency\n";
    printAdjacency(graph);

    std::cout << "\nEdge list\n";
    printEdges(graph);

    std::cout << "\nGuards\n";
    std::cout << std::boolalpha;
    std::cout << "duplicate edge:  " << graph.addEdge(provider, router1) << "  (expected: false)\n";
    std::cout << "self-loop:       " << graph.addEdge(router1, router1)  << "  (expected: false)\n";

    std::cout << "\nRemove edge router1 - router2\n";
    graph.removeEdge(router1, router2);
    printAdjacency(graph);
    std::cout << "edge count: " << graph.edges().size() << "  (expected: 3)\n";

    std::cout << "\nRemove node router2\n";
    graph.removeNode(router2);
    printAdjacency(graph);
    std::cout << "node count: " << graph.nodes().size() << "  (expected: 4)\n";
    std::cout << "edge count: " << graph.edges().size() << "  (expected: 1)\n";

    return 0;
}
