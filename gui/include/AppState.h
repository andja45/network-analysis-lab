#ifndef NETWORKANALYSISLAB_APPSTATE_H
#define NETWORKANALYSISLAB_APPSTATE_H

#include "graph/Graph.h"
#include "reachability/ReachabilityAnalyzer.h"
#include "routing/RoutingAnalyzer.h"
#include "routing/Metrics.h"
#include <optional>

enum class ViewMode { Neutral, BFS, Bridges, Routing, DCI };
enum class AnimationState { Idle, Running, Done };
enum class AlgoChoice { Dijkstra, AStar };
enum class MetricChoice { Fastest, Cheapest, LeastLoaded, MostReliable, Balanced };

struct RoutingPanelState {
    AlgoChoice algo = AlgoChoice::Dijkstra;
    MetricChoice metric = MetricChoice::Fastest;
    RoutingResult result;
    DCIResult dciResult;

    AnimationState animState = AnimationState::Idle;
    int animStep = 0;

    bool packetPhase = false;
    float packetT = 0.0f; // (0.0 to 1.0) progress along the current edge
    int packetEdgeIdx = 0; // index of the edge in path the packet is on
};

struct AppState {
    Graph graph;
    ReachabilityResult result;
    ViewMode viewMode = ViewMode::Neutral;
    AnimationState animationState = AnimationState::Idle;

    std::optional<NodeType> selectedType;
    int pendingEdgeFrom = -1;
    int draggedNode = -1;

    int animationStep = 0;
    float stepDelay = 0.4f;
    float timeSinceStep = 0.0f;
    int maxHops = -1;

    int routingSrc = -1;
    int routingDst = -1;
    float pendingLatency = 1.0f;
    float pendingPrice = 1.0f;
    float pendingBandwidth = 1000.0f;
    float pendingLoad = 0.0f;
    float pendingReliability = 1.0f;

    RoutingPanelState topPanel;
    RoutingPanelState bottomPanel;

    void runAnalysis();
    void runRouting(Heuristic h);
    void runResilience();
};

#endif
