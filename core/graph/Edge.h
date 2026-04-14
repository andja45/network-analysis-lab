#ifndef NETWORKANALYSISLAB_EDGE_H
#define NETWORKANALYSISLAB_EDGE_H

#include <functional>

struct Edge {
    int from;
    int to;
    float latency = 1.0f; // propagation delay in ms
    float price = 1.0f; // transfer cost
    float bandwidth = 1000.0f; // link capacity in Mbps
    float load = 0.0f; // current utilization ratio (0.0 = idle, 1.0 = saturated)
    float reliability = 1.0f; // probability link is up (used as -log(r))
    bool operator==(const Edge& o) const { return from == o.from && to == o.to; }
};

template<>
struct std::hash<Edge> {
    size_t operator()(const Edge& e) const {
        return std::hash<int>{}(e.from) ^ (std::hash<int>{}(e.to) << 1); // shift to avoid (u,v) and (v,u) clashing
    }
};

#endif //NETWORKANALYSISLAB_EDGE_H
