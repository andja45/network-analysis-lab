#include "examples/GraphExamples.h"
#include "reachability/ReachabilityAnalyzer.h"
#include "routing/RoutingAnalyzer.h"
#include "routing/Metrics.h"
#include <iostream>
#include <iomanip>
#include <limits>

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

static void printReachability(const Graph& graph, int maxHops) {
    ReachabilityResult r = analyze(graph, maxHops);

    int nProv = 0, nRouter = 0, nHost = 0;
    for (const auto& [id, node] : graph.nodes()) {
        if (node.type == NodeType::Provider)    ++nProv;
        else if (node.type == NodeType::Router) ++nRouter;
        else                                    ++nHost;
    }
    std::cout << "  " << nProv << " providers, " << nRouter << " routers, " << nHost << " hosts, "
              << graph.edges().size() << " edges\n";

    std::string hopStr = (maxHops != -1) ? " (max " + std::to_string(maxHops) + " hops)" : "";
    std::cout << "\n  Reachability" << hopStr << "\n";
    std::cout << "    Reachable:   " << r.reachableHosts << "/" << r.totalHosts << "\n";

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
    for (const auto& [e, c] : r.connectionCriticality) {
        if (c == ConnectionCriticality::Critical)          crits.push_back(e);
        else if (c == ConnectionCriticality::SemiCritical) semis.push_back(e);
        else                                               redundants.push_back(e);
    }
    std::cout << "\n  Connection Criticality\n";
    std::cout << "    Critical:  " << crits.size()      << "  "; printEdgeList(graph, crits);      std::cout << "\n";
    std::cout << "    SemiCrit:  " << semis.size()      << "  "; printEdgeList(graph, semis);      std::cout << "\n";
    std::cout << "    Redundant: " << redundants.size() << "  "; printEdgeList(graph, redundants); std::cout << "\n";
}

static const char* presetName(int i) {
    switch (i) {
        case 0: return "Fastest     ";
        case 1: return "Cheapest    ";
        case 2: return "LeastLoaded ";
        case 3: return "MostReliable";
        default: return "Balanced    ";
    }
}

static Metric presetMetric(int i) {
    switch (i) {
        case 0: return costFastest();
        case 1: return costCheapest();
        case 2: return costLeastLoaded();
        case 3: return costMostReliable();
        default: return costBalanced();
    }
}

static void printRouting(const Graph& graph, int src, int dst) {
    std::cout << "\n  Routing  " << label(graph, src) << " -> " << label(graph, dst) << "\n";

    Heuristic zero = [](int, int) { return 0.0f; };

    for (int i = 0; i < 5; i++) {
        Metric m = presetMetric(i);
        DijkstraResult dij = runDijkstra(graph, src, dst, m);
        AStarResult    ast = runAStar(graph, src, dst, m, zero);

        std::cout << "    " << presetName(i);
        if (!dij.found) {
            std::cout << "  no path\n";
            continue;
        }
        std::cout << "  cost=" << std::fixed << std::setprecision(2) << dij.totalCost << "  path: ";
        for (size_t j = 0; j < dij.path.size(); j++) {
            if (j) std::cout << "->";
            std::cout << label(graph, dij.path[j]);
        }
        std::cout << "  [D: relax=" << dij.relaxations << " vis=" << dij.visited
                  << "  A*: relax=" << ast.relaxations << " vis=" << ast.visited << "]\n";
    }

    DCIResult dci = computeDCI(graph, src, dst, costFastest(), 2.0f);

    int bridges = 0, crits = 0, semis = 0, redundants = 0;
    for (const auto& [e, dc] : dci.detourCriticality) {
        if      (dc == DetourCriticality::Bridge)       ++bridges;
        else if (dc == DetourCriticality::Critical)     ++crits;
        else if (dc == DetourCriticality::SemiCritical) ++semis;
        else                                            ++redundants;
    }
    std::cout << "\n  DCI (Fastest, threshold=2.0)  "
              << "Bridge=" << bridges << "  Critical=" << crits
              << "  SemiCrit=" << semis << "  Redundant=" << redundants << "\n";

    for (const auto& [e, ratio] : dci.dci) {
        const char* cls;
        switch (dci.detourCriticality.at(e)) {
            case DetourCriticality::Bridge:       cls = "bridge";    break;
            case DetourCriticality::Critical:     cls = "critical";  break;
            case DetourCriticality::SemiCritical: cls = "semi";      break;
            default:                              cls = "redundant"; break;
        }
        std::cout << "    " << label(graph, e.from) << "--" << label(graph, e.to);
        if (ratio == std::numeric_limits<float>::infinity())
            std::cout << "  DCI=inf   " << cls << "\n";
        else
            std::cout << "  DCI=" << std::fixed << std::setprecision(2) << ratio << "  " << cls << "\n";
    }
}

static void report(const std::string& name, const Graph& graph, int src, int dst, int maxHops) {
    std::cout << "\n*** " << name << " ***\n";
    printReachability(graph, maxHops);
    printRouting(graph, src, dst);
}

int main() {
    report("Crossroads",   makeCrossroads(),   0, 5,  3); // P1(0) -> H1(5)
    report("City Ring",    makeCityRing(),     0, 8,  3); // P1(0) -> H1(8)
    report("Dual ISP", makeDualISP(),  0, 14, 4); // P1(0) -> H3(14)
    std::cout << "\n";
}
