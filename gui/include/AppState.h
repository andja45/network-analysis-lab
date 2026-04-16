#ifndef NETWORKANALYSISLAB_APPSTATE_H
#define NETWORKANALYSISLAB_APPSTATE_H

#include "graph/Graph.h"
#include "reachability/ReachabilityAnalyzer.h"
#include "routing/RoutingAnalyzer.h"
#include "routing/Metrics.h"
#include <optional>
#include <limits>

enum class PanelMode { Reachability, Routing };
enum class ViewMode { Neutral, BFS, Bridges, Routing, DCI };
enum class AnimationState { Idle, Running, Done };
enum class AlgoChoice { Dijkstra, AStar };
enum class MetricChoice { Fastest, Cheapest, LeastLoaded, MostReliable, Balanced };

inline Metric toMetric(MetricChoice p) {
    switch (p) {
        case MetricChoice::Fastest:     return costFastest();
        case MetricChoice::Cheapest:    return costCheapest();
        case MetricChoice::LeastLoaded: return costLeastLoaded();
        case MetricChoice::MostReliable:return costMostReliable();
        default:                        return costBalanced();
    }
}

struct RoutingCanvasState {
    AlgoChoice algo = AlgoChoice::Dijkstra;
    MetricChoice metric = MetricChoice::Fastest;
    RoutingResult result;
    DCIResult dciResult;
    AnimationState animationState = AnimationState::Idle;
    int animationStep = 0;
    bool packetPhase = false;
    float packetT = 0.0f;     // progress along current edge (0.0 to 1.0)
    int packetEdgeIdx = 0;    // which edge in path the packet is on
};

struct AppState {
    Graph graph;
    ReachabilityResult result;
    PanelMode panelMode = PanelMode::Reachability;
    ViewMode viewMode = ViewMode::Neutral;
    AnimationState animationState = AnimationState::Idle;
    bool splitView = false;

    std::optional<NodeType> selectedType;
    int pendingEdgeFrom = -1;
    int draggedNode = -1;

    int animationStep = 0;
    float stepDelay = 0.4f;
    float timeSinceStep = 0.0f;
    int maxHops = -1;
    float maxDCI = 2.0f;

    int routingSrc = -1;
    int routingDst = -1;
    bool pickingSource = false;
    bool pickingDest = false;

    float pendingLatency = 1.0f;
    float pendingPrice = 1.0f;
    float pendingBandwidth = 1000.0f;
    float pendingLoad = 0.0f;
    float pendingReliability = 1.0f;

    RoutingCanvasState topCanvas;
    RoutingCanvasState bottomCanvas;

    void runAnalysis();
    void runRouting(Heuristic hTop, Heuristic hBot);
    void runResilience();
};

#endif
