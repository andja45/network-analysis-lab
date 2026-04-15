#include "AppState.h"

void AppState::runAnalysis() {
    result = analyze(graph, maxHops);
}

static Metric metricForPreset(MetricChoice p) {
    switch (p) {
        case MetricChoice::Fastest: return costFastest();
        case MetricChoice::Cheapest: return costCheapest();
        case MetricChoice::LeastLoaded: return costLeastLoaded();
        case MetricChoice::MostReliable: return costMostReliable();
        default: return costBalanced();
    }
}

void AppState::runRouting(Heuristic h) {
    topPanel.result = ::runRouting(graph, routingSrc, routingDst, metricForPreset(topPanel.metric), h);
    bottomPanel.result = ::runRouting(graph, routingSrc, routingDst, metricForPreset(bottomPanel.metric), h);
    topPanel.animState = AnimationState::Running;
    topPanel.animStep = 0;
    topPanel.packetPhase = false;
    topPanel.packetT = 0.0f;
    topPanel.packetEdgeIdx = 0;
    bottomPanel.animState = AnimationState::Running;
    bottomPanel.animStep = 0;
    bottomPanel.packetPhase = false;
    bottomPanel.packetT = 0.0f;
    bottomPanel.packetEdgeIdx = 0;
    viewMode = ViewMode::Routing;
}

void AppState::runResilience() {
    topPanel.dciResult = computeDCI(graph, routingSrc, routingDst, metricForPreset(topPanel.metric), 2.0f);
    bottomPanel.dciResult = computeDCI(graph, routingSrc, routingDst, metricForPreset(bottomPanel.metric), 2.0f);
    viewMode = ViewMode::DCI;
}
