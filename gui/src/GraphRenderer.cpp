#include "GraphRenderer.h"
#include "AppTheme.h"
#include <algorithm>
#include <cmath>

static float distToSegment(ImVec2 p, ImVec2 a, ImVec2 b) {
    float dx = b.x - a.x, dy = b.y - a.y;
    float len2 = dx*dx + dy*dy;
    if (len2 == 0) return hypotf(p.x - a.x, p.y - a.y);
    float t = std::clamp(((p.x-a.x)*dx + (p.y-a.y)*dy) / len2, 0.f, 1.f);
    return hypotf(p.x - (a.x + t*dx), p.y - (a.y + t*dy));
}

static ImVec2 nodePos(const std::vector<NodeVisual>& nodes, int id) {
    for (const auto& n : nodes)
        if (n.id == id) return n.pos;
    return {0, 0};
}

static const PathResult& activeResult(const RoutingCanvasState& c) {
    return c.algo == AlgoChoice::Dijkstra ? c.result.dijkstra : c.result.astar;
}
static ImU32 pathColor(const RoutingCanvasState& c) {
    return c.algo == AlgoChoice::Dijkstra ? AppTheme::PATH_DIJKSTRA : AppTheme::PATH_ASTAR;
}
static ImU32 visitColor(const RoutingCanvasState& c) {
    return c.algo == AlgoChoice::Dijkstra ? AppTheme::DIJKSTRA_VISITED : AppTheme::ASTAR_VISITED;
}

void GraphRenderer::addNode(int id, ImVec2 pos) {
    m_nodes.push_back({id, pos, AppTheme::NODE_DEFAULT});
}

void GraphRenderer::addEdge(const Edge& e) {
    m_edges.push_back({e, AppTheme::EDGE_DEFAULT, 2.0f});
}

void GraphRenderer::setNodePosition(int id, ImVec2 pos) {
    for (auto& n : m_nodes)
        if (n.id == id) { n.pos = pos; return; }
}

void GraphRenderer::removeNode(int id) {
    m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(), [id](const NodeVisual& n){ return n.id == id; }), m_nodes.end());
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), [id](const EdgeVisual& e){ return e.edge.from == id || e.edge.to == id; }), m_edges.end());
}

void GraphRenderer::removeEdge(const Edge& e) {
    auto rm = [&](const EdgeVisual& ev) { return ev.edge.from == e.from && ev.edge.to == e.to; };
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), rm), m_edges.end());
}

void GraphRenderer::rebuildProviderColors(const Graph& graph) {
    m_providerColors.clear();
    int idx = 0;
    for (const auto& [id, node] : graph.nodes())
        if (node.type == NodeType::Provider)
            m_providerColors[id] = AppTheme::PROVIDER_PALETTE[idx++ % AppTheme::PALETTE_SIZE];
}

void GraphRenderer::reset(const AppState& state) {
    m_nodes.clear(); m_edges.clear(); m_animationSteps.clear(); m_providerColors.clear();
    rebuildProviderColors(state.graph);
    for (const auto& [id, _] : state.graph.nodes()) addNode(id, {0, 0});
    for (const Edge& e : state.graph.edges()) addEdge(e);
}

void GraphRenderer::resetColors() {
    for (auto& n : m_nodes) n.color = AppTheme::NODE_DEFAULT;
    for (auto& e : m_edges) { e.color = AppTheme::EDGE_DEFAULT; e.thickness = 2.0f; }
}

void GraphRenderer::clear() {
    m_animationSteps.clear();
    resetColors();
}

void GraphRenderer::buildBFSAnimation(const AppState& state) {
    clear();
    rebuildProviderColors(state.graph);

    const auto& bfs = state.result.bfsResult;

    if (!bfs.bfsLevels.empty())
        for (int id : bfs.bfsLevels[0])
            m_animationSteps.push_back({{ {id, {}, m_providerColors.at(id), 2.0f} }});

    int maxLevel = (state.maxHops != -1)
        ? std::min(state.maxHops, (int)bfs.bfsLevels.size() - 1)
        : (int)bfs.bfsLevels.size() - 1;

    for (int d = 1; d <= maxLevel; ++d) {
        std::vector<ColorChange> step;
        for (int v : bfs.bfsLevels[d]) {
            int prov = bfs.nearestProvider.at(v);
            ImU32 color = m_providerColors.at(prov);
            int par = bfs.parent.at(v);
            step.push_back({ -1, {par, v}, color, 2.0f });
            ImU32 nodeColor = (state.graph.getNode(v).type == NodeType::Router) ? AppTheme::ROUTER : color;
            step.push_back({ v, {}, nodeColor, 2.0f });
        }
        if (!step.empty()) m_animationSteps.push_back(step);
    }

    if (!state.result.underservedHosts.empty()) {
        std::vector<ColorChange> step;
        for (int id : state.result.underservedHosts)
            step.push_back({ id, {}, AppTheme::SEMICRIT, 2.0f });
        m_animationSteps.push_back(step);
    }

    for (int id : state.result.unreachableHosts)
        m_animationSteps.push_back({{ {id, {}, AppTheme::UNREACHABLE, 2.0f} }});
}

void GraphRenderer::buildBridgeAnimation(const AppState& state) {
    clear();
    auto push1 = [&](const Edge& e, ImU32 c, float t){ m_animationSteps.push_back({{ {-1, e, c, t} }}); };
    for (const auto& [e, crit] : state.result.connectionCriticality)
        if (crit == ConnectionCriticality::Critical) push1(e, AppTheme::CRITICAL, 3.5f);
    for (const auto& [e, crit] : state.result.connectionCriticality)
        if (crit == ConnectionCriticality::SemiCritical) push1(e, AppTheme::SEMICRIT, 2.5f);
    std::vector<ColorChange> red;
    for (const auto& [e, crit] : state.result.connectionCriticality)
        if (crit == ConnectionCriticality::Redundant) red.push_back({-1, e, AppTheme::REDUNDANT, 2.0f});
    if (!red.empty()) m_animationSteps.push_back(red);
}

void GraphRenderer::applyStep(int stepIdx) {
    for (const ColorChange& c : m_animationSteps[stepIdx]) {
        if (c.nodeId != -1) {
            for (auto& n : m_nodes)
                if (n.id == c.nodeId) { n.color = c.color; break; }
        } else {
            for (auto& e : m_edges)
                if ((e.edge.from == c.edge.from && e.edge.to == c.edge.to) ||
                    (e.edge.from == c.edge.to && e.edge.to == c.edge.from)) {
                    e.color = c.color;
                    e.thickness = c.thickness;
                    break;
                }
        }
    }
}

bool GraphRenderer::step(int& stepCounter) {
    if (stepCounter >= (int)m_animationSteps.size()) return false;
    applyStep(stepCounter++);
    return stepCounter < (int)m_animationSteps.size();
}

void GraphRenderer::moveNode(int id, ImVec2 delta) {
    for (auto& n : m_nodes)
        if (n.id == id) { n.pos.x += delta.x; n.pos.y += delta.y; return; }
}

ImVec2 GraphRenderer::nodePosition(int id) const {
    return nodePos(m_nodes, id);
}

int GraphRenderer::nodeAt(ImVec2 p) const {
    for (const auto& n : m_nodes)
        if (hypotf(p.x - n.pos.x, p.y - n.pos.y) < 18.0f) return n.id;
    return -1;
}

Edge GraphRenderer::edgeAt(ImVec2 p) const {
    for (const auto& e : m_edges) {
        ImVec2 a = nodePos(m_nodes, e.edge.from);
        ImVec2 b = nodePos(m_nodes, e.edge.to);
        if (distToSegment(p, a, b) < 6.0f) return e.edge;
    }
    return {-1, -1};
}

void GraphRenderer::drawEdge(ImDrawList* dl, const EdgeVisual& ev, MetricChoice metric, bool showLabel) const {
    ImVec2 a = nodePos(m_nodes, ev.edge.from);
    ImVec2 b = nodePos(m_nodes, ev.edge.to);
    dl->AddLine(a, b, ev.color, ev.thickness);
    if (!showLabel) return;
    ImVec2 mid = {(a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f};
    float dx = b.x - a.x, dy = b.y - a.y;
    float len = hypotf(dx, dy);
    float ox = (len > 0) ? -dy / len * 10.0f : 0.0f;
    float oy = (len > 0) ? dx / len * 10.0f : 0.0f;
    char buf[16];
    const Edge& e = ev.edge;
    switch (metric) {
        case MetricChoice::Fastest: std::snprintf(buf, 16, "%.0fms", e.latency); break;
        case MetricChoice::Cheapest: std::snprintf(buf, 16, "$%.1f", e.price); break;
        case MetricChoice::LeastLoaded: std::snprintf(buf, 16, "%.2f", e.load); break;
        case MetricChoice::MostReliable: std::snprintf(buf, 16, "%.2f", e.reliability); break;
        default: {
            float score = 0.4f * e.latency + 0.4f * e.price
                        + 0.2f * (-std::log(std::max(e.reliability, 1e-6f)));
            std::snprintf(buf, 16, "%.2f", score);
            break;
        }
    }
    ImVec2 ts = ImGui::CalcTextSize(buf);
    dl->AddText({mid.x + ox - ts.x / 2.0f, mid.y + oy - ts.y / 2.0f}, AppTheme::LABEL, buf);
}

void GraphRenderer::drawNode(ImDrawList* dl, const NodeVisual& nv, const AppState& state) const {
    ImVec2 p = nv.pos;
    ImU32 col = nv.color;
    NodeType type = state.graph.getNode(nv.id).type;

    if (type == NodeType::Provider) {
        float w = 14, h = 22;
        dl->AddRectFilled({p.x - w/2, p.y - h/2}, {p.x + w/2, p.y + h/2}, col, 2.0f);
        dl->AddLine({p.x - 4, p.y - h/2}, {p.x - 4, p.y - h/2 - 8}, col, 1.5f);
        dl->AddLine({p.x + 4, p.y - h/2}, {p.x + 4, p.y - h/2 - 8}, col, 1.5f);
    } else if (type == NodeType::Router) {
        float r = 14, t = 5;
        dl->AddQuadFilled({p.x, p.y-r}, {p.x+r, p.y}, {p.x, p.y+r}, {p.x-r, p.y}, col);
        dl->AddLine({p.x, p.y-r-t}, {p.x, p.y-r+t}, col, 1.5f);
        dl->AddLine({p.x, p.y+r-t}, {p.x, p.y+r+t}, col, 1.5f);
        dl->AddLine({p.x-r-t, p.y}, {p.x-r+t, p.y}, col, 1.5f);
        dl->AddLine({p.x+r-t, p.y}, {p.x+r+t, p.y}, col, 1.5f);
    } else {
        float w = 18, h = 14;
        dl->AddRectFilled({p.x - w/2, p.y - h/2}, {p.x + w/2, p.y + h/2}, col, 2.0f);
        dl->AddLine({p.x, p.y + h/2}, {p.x, p.y + h/2 + 6}, col, 2.0f);
        dl->AddLine({p.x - 5, p.y + h/2 + 6}, {p.x + 5, p.y + h/2 + 6}, col, 2.0f);
    }

    const auto& label = state.graph.getNode(nv.id).label;
    dl->AddText({p.x - ImGui::CalcTextSize(label.c_str()).x / 2, p.y + 20}, AppTheme::LABEL, label.c_str());
}

void GraphRenderer::draw(ImDrawList* dl, ImVec2 origin, ImVec2 canvasSize,
                          const AppState& state, MetricChoice metric, bool showEdgeLabels) {
    for (auto& n : m_nodes) n.pos = {n.pos.x + origin.x, n.pos.y + origin.y};
    for (const auto& e : m_edges) drawEdge(dl, e, metric, showEdgeLabels);
    for (const auto& n : m_nodes) drawNode(dl, n, state);
    for (auto& n : m_nodes) n.pos = {n.pos.x - origin.x, n.pos.y - origin.y};
}

void GraphRenderer::buildDCIAnimation(const RoutingCanvasState& canvas) {
    clear();
    auto push1 = [&](const Edge& e, ImU32 c, float t){ m_animationSteps.push_back({{ {-1, e, c, t} }}); };
    for (const auto& [e, dc] : canvas.dciResult.detourCriticality)
        if (dc == DetourCriticality::Bridge) push1(e, AppTheme::CRITICAL, 3.5f);
    for (const auto& [e, dc] : canvas.dciResult.detourCriticality)
        if (dc == DetourCriticality::Critical) push1(e, AppTheme::SEMICRIT, 3.0f);
    std::vector<ColorChange> rest;
    for (const auto& [e, dc] : canvas.dciResult.detourCriticality) {
        if (dc == DetourCriticality::SemiCritical) rest.push_back({-1, e, AppTheme::SEMICRIT, 2.5f});
        else if (dc == DetourCriticality::Redundant) rest.push_back({-1, e, AppTheme::REDUNDANT, 2.0f});
    }
    if (!rest.empty()) m_animationSteps.push_back(rest);
}

void GraphRenderer::buildRoutingAnimation(const AppState& state, const RoutingCanvasState& canvas) {
    clear();

    for (int id : activeResult(canvas).visitOrder)
        m_animationSteps.push_back({{ {id, {}, visitColor(canvas), 2.0f} }});

    const auto& path = activeResult(canvas).path;
    std::vector<ColorChange> pathStep;
    for (int i = 0; i + 1 < (int)path.size(); ++i)
        pathStep.push_back({ -1, {path[i], path[i+1]}, pathColor(canvas), 3.5f });
    for (int id : path)
        pathStep.push_back({ id, {}, pathColor(canvas), 2.0f });
    if (!pathStep.empty())
        m_animationSteps.push_back(pathStep);
}

void GraphRenderer::advancePacket(RoutingCanvasState& canvas, float dt, float speed) {
    const auto& path = activeResult(canvas).path;
    if (canvas.packetEdgeIdx >= (int)path.size() - 1) return;
    if ((canvas.packetT += dt * speed) >= 1.0f) { canvas.packetT = 0.0f; ++canvas.packetEdgeIdx; }
}

void GraphRenderer::drawPacket(ImDrawList* dl, ImVec2 origin, const RoutingCanvasState& canvas) {
    const auto& path = activeResult(canvas).path;
    if (canvas.packetEdgeIdx >= (int)path.size() - 1) return;
    ImVec2 a = nodePosition(path[canvas.packetEdgeIdx]);
    ImVec2 b = nodePosition(path[canvas.packetEdgeIdx + 1]);
    float t = canvas.packetT;
    dl->AddCircleFilled({origin.x + a.x + (b.x-a.x)*t, origin.y + a.y + (b.y-a.y)*t}, 8.0f, pathColor(canvas));
}
