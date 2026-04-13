#include "AppState.h"

void AppState::runAnalysis() {
    result = analyze(graph, maxHops);
}
