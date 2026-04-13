#ifndef NETWORKREACHABILITYANALYSIS_APPSTATE_H
#define NETWORKREACHABILITYANALYSIS_APPSTATE_H

#include "Graph.h"
#include "Analyzer.h"

enum class ViewMode { Neutral, BFS, Bridges };
enum class AnimationState { Idle, Running, Done };

struct AppState {
    Graph graph;
    AnalysisResult result;

    ViewMode viewMode = ViewMode::Neutral;
    AnimationState animationState = AnimationState::Idle;
    int animationStep = 0;
    float stepDelay = 0.4f;
    float timeSinceStep = 0.0f;

    int maxHops = -1;

    NodeType selectedType = NodeType::Provider;
    int pendingEdgeFrom = -1; // when second node is selected, edge is created
    int draggedNode = -1;

    void runAnalysis();
};

#endif
