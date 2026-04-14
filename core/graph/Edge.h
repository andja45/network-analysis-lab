#ifndef NETWORKREACHABILITYANALYSIS_EDGE_H
#define NETWORKREACHABILITYANALYSIS_EDGE_H

#include <functional>

struct Edge {
    int from;
    int to;
    bool operator ==(const Edge& o) const { return from == o.from && to == o.to; }
};

template<>
struct std::hash<Edge> {
    size_t operator()(const Edge& e) const {
        return std::hash<int>{}(e.from) ^ (std::hash<int>{}(e.to) << 1); // shift to avoid (u,v) and (v,u) clashing
    }
};

#endif //NETWORKREACHABILITYANALYSIS_EDGE_H
