#include "GraphExamples.h"
#include "Analyzer.h"
#include <iostream>

static std::string nodeStr(const Graph& graph, int id) {
    return graph.getNode(id).label + "(" + std::to_string(id) + ")";
}

static void printReport(const std::string& name, const Graph& graph, const AnalysisResult& result) {
    std::cout << "\n" << name << "\n";

    std::cout << "Hosts: "
              << result.totalHosts << " total | "
              << result.reachableHosts << " reachable | "
              << (result.totalHosts - result.reachableHosts) << " unreachable\n";

    std::cout << "Unreachable: [";
    for (size_t i = 0; i < result.unreachableHosts.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << nodeStr(graph, result.unreachableHosts[i]);
    }
    std::cout << "]\n";

    std::cout << "Underserved: [";
    for (size_t i = 0; i < result.underservedHosts.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << nodeStr(graph, result.underservedHosts[i]);
    }
    std::cout << "]\n";

    std::cout << "Bridges: " << result.bridgeImpacts.size() << "\n";
    for (const BridgeImpact& impact : result.bridgeImpacts) {
        std::cout << "  " << nodeStr(graph, impact.bridge.from)
                  << " -- "
                  << nodeStr(graph, impact.bridge.to)
                  << "  =>  disconnects: [";

        for (size_t i = 0; i < impact.disconnectedHosts.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << nodeStr(graph, impact.disconnectedHosts[i]);
        }
        std::cout << "]\n";
    }
}

int main() {
    const int maxHops = 3;

    Graph office = makeOfficeNetwork();
    Graph redundant = makeRedundantNetwork();
    Graph linear = makeLinearNetwork();

    printReport("Office Network", office, analyze(office, maxHops));
    printReport("Redundant Network", redundant, analyze(redundant, maxHops));
    printReport("Linear Network", linear, analyze(linear, maxHops));

    return 0;
}
