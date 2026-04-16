#include "AppState.h"

void AppState::runAnalysis() {
    result = analyze(graph, maxHops);
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
        c.dciResult = computeDCI(graph, routingSrc, routingDst, toMetric(c.metric), maxDCI);
        c.animationState = AnimationState::Running;
        c.animationStep = 0;
    };
    run(topCanvas); run(bottomCanvas);
    viewMode = ViewMode::DCI;
}
