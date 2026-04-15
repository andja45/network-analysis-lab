#include "AppState.h"

void AppState::runAnalysis() {
    result = analyze(graph, maxHops);
}

static Metric toMetric(MetricChoice p) {
    switch (p) {
        case MetricChoice::Fastest:     return costFastest();
        case MetricChoice::Cheapest:    return costCheapest();
        case MetricChoice::LeastLoaded: return costLeastLoaded();
        case MetricChoice::MostReliable:return costMostReliable();
        default:                        return costBalanced();
    }
}

void AppState::runRouting(Heuristic h) {
    auto run = [&](RoutingCanvasState& c) {
        c.result = ::runRouting(graph, routingSrc, routingDst, toMetric(c.metric), h);
        c.animationState = AnimationState::Running;
        c.animationStep = 0;
        c.packetPhase = false;
        c.packetT = 0.0f;
        c.packetEdgeIdx = 0;
        c.dciResult = {};
    };
    run(topCanvas);
    run(bottomCanvas);
    viewMode = ViewMode::Routing;
}

void AppState::runResilience() {
    auto run = [&](RoutingCanvasState& c) {
        c.dciResult = computeDCI(graph, routingSrc, routingDst, toMetric(c.metric), 2.0f);
        c.animationState = AnimationState::Running;
        c.animationStep = 0;
    };
    run(topCanvas); run(bottomCanvas);
    viewMode = ViewMode::DCI;
}
