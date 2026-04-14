#include "examples/GraphExamples.h"
#include "reachability/Analyzer.h"
#include <iostream>

static std::string label(const Graph& g, int id) {
    return g.getNode(id).label;
}

static void printEdgeList(const Graph& g, const std::vector<Edge>& edges) {
    std::cout << "[";
    for (size_t i = 0; i < edges.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << label(g, edges[i].from) << "--" << label(g, edges[i].to);
    }
    std::cout << "]";
}

static void printReport(const std::string& name, const Graph& graph, int maxHops) {
    AnalysisResult r = analyze(graph, maxHops);

    std::cout << "\n***" << name << ": ***" << "\n";

    int nProv = 0, nRouter = 0, nHost = 0;
    for (const auto& [id, node] : graph.nodes()) {
        if (node.type == NodeType::Provider) ++nProv;
        else if (node.type == NodeType::Router) ++nRouter;
        else ++nHost;
    }
    std::cout << "  Nodes: " << nProv << " providers, " << nRouter << " routers, " << nHost << " hosts\n";
    std::cout << "  Edges: " << graph.edges().size() << "\n";

    std::string hopStr = (maxHops != -1) ? " (max " + std::to_string(maxHops) + " hops)" : "";
    std::cout << "\n  Reachability" << hopStr << "\n";
    std::cout << "    Reachable:   " << r.reachableHosts << " / " << r.totalHosts << "\n";

    std::cout << "    Unreachable: " << r.unreachableHosts.size();
    if (!r.unreachableHosts.empty()) {
        std::cout << "  [";
        for (size_t i = 0; i < r.unreachableHosts.size(); i++) {
            if (i) std::cout << ", ";
            std::cout << label(graph, r.unreachableHosts[i]);
        }
        std::cout << "]";
    }
    std::cout << "\n";

    if (maxHops != -1) {
        std::cout << "    Underserved: " << r.underservedHosts.size();
        if (!r.underservedHosts.empty()) {
            std::cout << "  [";
            for (size_t i = 0; i < r.underservedHosts.size(); i++) {
                if (i) std::cout << ", ";
                std::cout << label(graph, r.underservedHosts[i]);
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }

    std::vector<Edge> crits, semis, redundants;
    for (const auto& [e, c] : r.edgeCriticality) {
        if (c == EdgeCriticality::Critical)     crits.push_back(e);
        else if (c == EdgeCriticality::SemiCritical) semis.push_back(e);
        else redundants.push_back(e);
    }

    std::cout << "\n  Connection Criticality\n";
    std::cout << "    Critical:    " << crits.size() << "  "; printEdgeList(graph, crits); std::cout << "\n";
    std::cout << "    Semi-crit:   " << semis.size() << "  "; printEdgeList(graph, semis); std::cout << "\n";
    std::cout << "    Redundant:   " << redundants.size() << "  "; printEdgeList(graph, redundants); std::cout << "\n";
}

int main() {
    const int maxHops = 3;
    printReport("Office Network",makeOfficeNetwork(), maxHops);
    printReport("Redundant Network",makeRedundantNetwork(), maxHops);
    printReport("Linear Network",makeLinearNetwork(), maxHops);
    std::cout << "\n";
}
