#include "GraphExamples.h"

// Two providers feeding into a redundant core ring (R1-R2), with bridge chains
// reaching out to hosts. Core is resilient -- branches are vulnerable.
// Tests: BFS multi-source, redundant core, bridge chains, underserved hosts, isolated node.
//
//   ProvA   ProvB
//     |       |
//     R1 --- R2
//     |       |
//     R3      R4 -- H2     (3 hops from ProvB -- ok)
//     |
//     R5 -- H1             (4 hops from ProvA -- underserved)
//     |
//     H3                   (4 hops from ProvA -- underserved)
//   H4                     (isolated -- unreachable)
Graph makeOfficeNetwork() {
    Graph g;

    int provA = g.addNode("ProvA", NodeType::Provider);
    int provB = g.addNode("ProvB", NodeType::Provider);
    int r1    = g.addNode("R1",    NodeType::Router);
    int r2    = g.addNode("R2",    NodeType::Router);
    int r3    = g.addNode("R3",    NodeType::Router);
    int r4    = g.addNode("R4",    NodeType::Router);
    int r5    = g.addNode("R5",    NodeType::Router);
    int h1    = g.addNode("H1",    NodeType::Host);
    int h2    = g.addNode("H2",    NodeType::Host);
    int h3    = g.addNode("H3",    NodeType::Host);
    g.addNode("H4", NodeType::Host); // isolated

    g.addEdge(provA, r1);
    g.addEdge(provB, r2);
    g.addEdge(r1,    r2); // core cross-link -- makes R1-R2 resilient
    g.addEdge(r1,    r3);
    g.addEdge(r2,    r4);
    g.addEdge(r3,    r5);
    g.addEdge(r5,    h1);
    g.addEdge(r5,    h3);
    g.addEdge(r4,    h2);

    return g;
}

// Two providers, fully redundant mesh — no bridges, all hosts within 3 hops.
// Tests: all hosts reachable, zero bridges, zero underserved.
//
//   ProvA -- R1 -- R2 -- H1
//    |    \   |  \  |
//   ProvB -- R3 -- R4 -- H2
//                   |
//                   H3
Graph makeRedundantNetwork() {
    Graph g;

    int provA = g.addNode("ProvA", NodeType::Provider);
    int provB = g.addNode("ProvB", NodeType::Provider);
    int r1    = g.addNode("R1",    NodeType::Router);
    int r2    = g.addNode("R2",    NodeType::Router);
    int r3    = g.addNode("R3",    NodeType::Router);
    int r4    = g.addNode("R4",    NodeType::Router);
    int h1    = g.addNode("H1",    NodeType::Host);
    int h2    = g.addNode("H2",    NodeType::Host);
    int h3    = g.addNode("H3",    NodeType::Host);

    g.addEdge(provA, r1);
    g.addEdge(provA, provB);
    g.addEdge(provA, r3);
    g.addEdge(provB, r3);
    g.addEdge(r1, r2);
    g.addEdge(r1, r3);
    g.addEdge(r1, r4);
    g.addEdge(r2, r4);
    g.addEdge(r3, r4);
    g.addEdge(r2, h1);
    g.addEdge(r4, h2);
    g.addEdge(r4, h3);

    return g;
}

// Single provider, pure chain — every edge is a bridge, H4 and H5 isolated.
// Tests: all edges are bridges, high bridge impact, unreachable hosts.
//
//   Prov -- R1 -- R2 -- R3 -- H1
//                  |
//                  R4 -- H2
//                  |
//                  H3
//   H4    H5       (isolated)
Graph makeLinearNetwork() {
    Graph g;

    int prov = g.addNode("Prov", NodeType::Provider);
    int r1   = g.addNode("R1",   NodeType::Router);
    int r2   = g.addNode("R2",   NodeType::Router);
    int r3   = g.addNode("R3",   NodeType::Router);
    int r4   = g.addNode("R4",   NodeType::Router);
    int h1   = g.addNode("H1",   NodeType::Host);
    int h2   = g.addNode("H2",   NodeType::Host);
    int h3   = g.addNode("H3",   NodeType::Host);
    g.addNode("H4", NodeType::Host); // isolated
    g.addNode("H5", NodeType::Host); // isolated

    g.addEdge(prov, r1);
    g.addEdge(r1, r2);
    g.addEdge(r2, r3);
    g.addEdge(r3, h1);
    g.addEdge(r2, r4);
    g.addEdge(r4, h2);
    g.addEdge(r4, h3);

    return g;
}
