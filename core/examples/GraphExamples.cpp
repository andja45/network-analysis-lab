#include "GraphExamples.h"

// Three paths from provider to host, each wins under a different cost metric. H2 connects only through R3.
Graph makeCrossroads() {
    Graph g;

    int p1 = g.addNode("P1", NodeType::Provider);
    int r1 = g.addNode("R1", NodeType::Router);
    int r2 = g.addNode("R2", NodeType::Router);
    int r3 = g.addNode("R3", NodeType::Router);
    int r4 = g.addNode("R4", NodeType::Router);
    int h1 = g.addNode("H1", NodeType::Host);
    int h2 = g.addNode("H2", NodeType::Host);

    g.addEdge(p1, r1, 0.5f,  1.0f, 10000.0f, 0.00f, 0.999f);
    g.addEdge(r1, r2, 1.0f, 10.0f,  1000.0f, 0.70f, 0.60f);
    g.addEdge(r2, h1, 1.0f, 10.0f,  1000.0f, 0.70f, 0.60f);
    g.addEdge(r1, r3, 8.0f,  1.0f,  1000.0f, 0.10f, 0.99f);
    g.addEdge(r3, h1, 8.0f,  1.0f,  1000.0f, 0.10f, 0.99f);
    g.addEdge(r1, r4, 4.0f,  4.0f,  1000.0f, 0.05f, 0.92f);
    g.addEdge(r4, h1, 4.0f,  4.0f,  1000.0f, 0.05f, 0.92f);
    g.addEdge(r3, h2, 2.0f,  2.0f,   500.0f, 0.00f, 0.99f);

    return g;
}

// Ring backbone with two providers — the ring means every router link has an alternative. The express shortcut R1-R4 is fastest but unreliable.
Graph makeCityRing() {
    Graph g;

    int p1 = g.addNode("P1", NodeType::Provider);
    int p2 = g.addNode("P2", NodeType::Provider);
    int r1 = g.addNode("R1", NodeType::Router);
    int r2 = g.addNode("R2", NodeType::Router);
    int r3 = g.addNode("R3", NodeType::Router);
    int r4 = g.addNode("R4", NodeType::Router);
    int r5 = g.addNode("R5", NodeType::Router);
    int r6 = g.addNode("R6", NodeType::Router);
    int h1 = g.addNode("H1", NodeType::Host);
    int h2 = g.addNode("H2", NodeType::Host);
    int h3 = g.addNode("H3", NodeType::Host);

    g.addEdge(p1, r1, 0.5f,  1.0f, 10000.0f, 0.00f, 0.999f);
    g.addEdge(p2, r4, 0.5f,  1.0f, 10000.0f, 0.00f, 0.999f);

    g.addEdge(r1, r2, 2.0f,  6.0f, 1000.0f, 0.65f, 0.97f);
    g.addEdge(r2, r3, 2.0f,  6.0f, 1000.0f, 0.65f, 0.97f);
    g.addEdge(r3, r4, 2.0f,  6.0f, 1000.0f, 0.65f, 0.97f);
    g.addEdge(r4, r5, 5.0f,  2.0f, 1000.0f, 0.10f, 0.98f);
    g.addEdge(r5, r6, 5.0f,  2.0f, 1000.0f, 0.10f, 0.98f);
    g.addEdge(r6, r1, 5.0f,  2.0f, 1000.0f, 0.10f, 0.98f);
    g.addEdge(r1, r4, 1.5f, 15.0f, 2000.0f, 0.00f, 0.72f);

    g.addEdge(r5, h1, 1.0f, 1.0f, 100.0f, 0.00f, 0.99f);
    g.addEdge(r5, h2, 1.0f, 1.0f, 100.0f, 0.00f, 0.99f);
    g.addEdge(r3, h3, 1.0f, 1.0f, 100.0f, 0.00f, 0.99f);

    return g;
}

// Two ISP rings linked by two cross-connections — fast+expensive vs slow+cheap. H5 is unreachable.
Graph makeISPBackbone() {
    Graph g;

    int p1  = g.addNode("P1",  NodeType::Provider);
    int p2  = g.addNode("P2",  NodeType::Provider);

    int r1  = g.addNode("R1",  NodeType::Router);
    int r2  = g.addNode("R2",  NodeType::Router);
    int r3  = g.addNode("R3",  NodeType::Router);
    int r4  = g.addNode("R4",  NodeType::Router);

    int r5  = g.addNode("R5",  NodeType::Router);
    int r6  = g.addNode("R6",  NodeType::Router);
    int r7  = g.addNode("R7",  NodeType::Router);

    int r8  = g.addNode("R8",  NodeType::Router);
    int r9  = g.addNode("R9",  NodeType::Router);
    int r10 = g.addNode("R10", NodeType::Router);

    int h1 = g.addNode("H1",  NodeType::Host);
    int h2 = g.addNode("H2",  NodeType::Host);
    int h3 = g.addNode("H3",  NodeType::Host);
    int h4 = g.addNode("H4",  NodeType::Host);
    g.addNode("H5", NodeType::Host);

    g.addEdge(p1, r1, 0.5f, 1.0f, 10000.0f, 0.00f, 0.999f);
    g.addEdge(p2, r6, 0.5f, 1.0f, 10000.0f, 0.00f, 0.999f);

    g.addEdge(r1, r2, 3.0f, 4.0f, 10000.0f, 0.40f, 0.97f);
    g.addEdge(r2, r3, 3.0f, 4.0f, 10000.0f, 0.40f, 0.97f);
    g.addEdge(r3, r4, 3.0f, 4.0f, 10000.0f, 0.40f, 0.97f);
    g.addEdge(r4, r1, 3.0f, 4.0f, 10000.0f, 0.40f, 0.97f);
    g.addEdge(r1, r3, 2.0f, 7.0f,  5000.0f, 0.10f, 0.93f);

    g.addEdge(r5, r6, 3.0f, 4.0f, 10000.0f, 0.30f, 0.98f);
    g.addEdge(r6, r7, 3.0f, 4.0f, 10000.0f, 0.30f, 0.98f);
    g.addEdge(r7, r5, 3.0f, 4.0f, 10000.0f, 0.30f, 0.98f);

    g.addEdge(r2, r5, 2.0f, 12.0f, 5000.0f, 0.20f, 0.96f);
    g.addEdge(r3, r7, 8.0f,  3.0f, 2000.0f, 0.10f, 0.98f);

    g.addEdge(r4,  r8,  1.0f, 1.0f, 1000.0f, 0.00f, 0.999f);
    g.addEdge(r3,  r9,  1.0f, 1.0f, 1000.0f, 0.00f, 0.999f);
    g.addEdge(r7,  r10, 1.0f, 1.0f, 1000.0f, 0.00f, 0.999f);

    g.addEdge(r8,  h1, 1.0f, 1.0f, 100.0f, 0.00f, 0.999f);
    g.addEdge(r9,  h2, 1.0f, 1.0f, 100.0f, 0.00f, 0.999f);
    g.addEdge(r10, h3, 1.0f, 1.0f, 100.0f, 0.00f, 0.999f);
    g.addEdge(r10, h4, 1.0f, 1.0f, 100.0f, 0.00f, 0.999f);

    return g;
}
