#ifndef NETWORKANALYSISLAB_APPSTATE_H
#define NETWORKANALYSISLAB_APPSTATE_H

#include "graph/Graph.h"
#include "reachability/ReachabilityAnalyzer.h"
#include <optional>

enum class ViewMode { Neutral, BFS, Bridges };
enum class AnimationState { Idle, Running, Done };

struct AppState {
    Graph graph;
    ReachabilityResult result;

    ViewMode viewMode = ViewMode::Neutral;
    AnimationState animationState = AnimationState::Idle;
    int animationStep = 0;
    float stepDelay = 0.4f;
    float timeSinceStep = 0.0f;

    int maxHops = -1;

    std::optional<NodeType> selectedType; // empty = no node tool active
    int pendingEdgeFrom = -1; // when second node is selected, edge is created
    int draggedNode = -1;

    void runAnalysis();
};

#endif
