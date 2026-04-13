#include "GraphRenderer.h"
#include "imgui.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>

static constexpr ImU32 COL_NODE_DEFAULT = IM_COL32( 80,  80,  80, 255);
static constexpr ImU32 COL_EDGE_DEFAULT = IM_COL32(100, 100, 100, 255);
static constexpr ImU32 COL_ROUTER      = IM_COL32(100, 100, 100, 255);
static constexpr ImU32 COL_UNREACHABLE = IM_COL32(220,  60,  60, 255);
static constexpr ImU32 COL_CRITICAL    = IM_COL32(220,  60,  60, 255);
static constexpr ImU32 COL_SEMICRIT    = IM_COL32(230, 140,  40, 255);
static constexpr ImU32 COL_REDUNDANT   = IM_COL32( 60, 180,  80, 255);
static constexpr ImU32 COL_DFS_FLASH   = IM_COL32(100, 160, 230, 255);

static const ImU32 PROVIDER_PALETTE[] = {
    IM_COL32( 80, 140, 220, 255),
    IM_COL32(140,  80, 220, 255),
    IM_COL32( 40, 180, 160, 255),
    IM_COL32(220, 160,  40, 255),
    IM_COL32(200,  80, 140, 255),
};
static constexpr int PALETTE_SIZE = 5;

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

void GraphRenderer::addNode(int id, ImVec2 pos) {
    m_nodes.push_back({id, pos, COL_NODE_DEFAULT});
}

void GraphRenderer::addEdge(const Edge& e) {
    m_edges.push_back({e, COL_EDGE_DEFAULT, 2.0f});
}

void GraphRenderer::removeNode(int id) {
    auto rm = [id](const NodeVisual& n) { return n.id == id; };
    m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(), rm), m_nodes.end());

    auto rmEdge = [id](const EdgeVisual& e) { return e.edge.from == id || e.edge.to == id; };
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), rmEdge), m_edges.end());
}

void GraphRenderer::removeEdge(const Edge& e) {
    auto rm = [&](const EdgeVisual& ev) { return ev.edge.from == e.from && ev.edge.to == e.to; };
    m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(), rm), m_edges.end());
}

void GraphRenderer::reset(const AppState& state, ImVec2 canvasSize) {
    m_nodes.clear();
    m_edges.clear();
    m_animationSteps.clear();
    m_providerColors.clear();
    computeLayout(state, canvasSize);
}

void GraphRenderer::computeLayout(const AppState& state, ImVec2 canvasSize) {
    int colorIdx = 0;
    for (auto& [id, node] : state.graph.nodes())
        if (node.type == NodeType::Provider)
            m_providerColors[id] = PROVIDER_PALETTE[colorIdx++ % PALETTE_SIZE];

    const auto& levels = state.result.bfs.bfsLevels;

    // nodes not in any BFS level are unreachable — place them in a bottom row
    std::unordered_set<int> placed;
    for (const auto& level : levels)
        for (int id : level) placed.insert(id);

    std::vector<int> unreachable;
    for (auto& [id, node] : state.graph.nodes())
        if (!placed.count(id)) unreachable.push_back(id);

    int rows = (int)levels.size() + (unreachable.empty() ? 0 : 1);
    float rowH = canvasSize.y / (float)(rows + 1);
    float row = rowH;

    for (const auto& level : levels) {
        float colW = canvasSize.x / (float)(level.size() + 1);
        float col = colW;
        for (int id : level) { addNode(id, {col, row}); col += colW; }
        row += rowH;
    }
    if (!unreachable.empty()) {
        float colW = canvasSize.x / (float)(unreachable.size() + 1);
        float col = colW;
        for (int id : unreachable) { addNode(id, {col, row}); col += colW; }
    }

    for (const Edge& e : state.graph.edges()) addEdge(e);
}

void GraphRenderer::resetColors(const AppState& state) {
    for (auto& n : m_nodes)
        n.color = COL_NODE_DEFAULT;
    for (auto& e : m_edges) {
        e.color = COL_EDGE_DEFAULT;
        e.thickness = 2.0f;
    }
}

void GraphRenderer::clear(const AppState& state) {
    m_animationSteps.clear();
    resetColors(state);
}

void GraphRenderer::buildBFSAnimation(const AppState& state) {
    m_animationSteps.clear();
    resetColors(state);

    m_providerColors.clear();
    int colorIdx = 0;
    for (const auto& [id, node] : state.graph.nodes())
        if (node.type == NodeType::Provider)
            m_providerColors[id] = PROVIDER_PALETTE[colorIdx++ % PALETTE_SIZE];

    const auto& bfs = state.result.bfs;

    // level 0: light up each provider in its color
    if (!bfs.bfsLevels.empty())
        for (int id : bfs.bfsLevels[0])
            m_animationSteps.push_back({{ {id, {}, m_providerColors.at(id), 2.0f} }});

    // each level = one wave step, stopping at maxHops if set
    int maxLevel = (state.maxHops != -1)
        ? std::min(state.maxHops, (int)bfs.bfsLevels.size() - 1)
        : (int)bfs.bfsLevels.size() - 1;

    for (int d = 1; d <= maxLevel; ++d) {
        std::vector<ColorChange> step;
        for (int v : bfs.bfsLevels[d]) {
            int prov  = bfs.nearestProvider.at(v);
            ImU32 color = m_providerColors.at(prov);
            int par   = bfs.parent.at(v);

            step.push_back({ -1, {par, v}, color, 2.0f });
            ImU32 nodeColor = (state.graph.getNode(v).type == NodeType::Router) ? COL_ROUTER : color;
            step.push_back({ v, {}, nodeColor, 2.0f });
        }
        if (!step.empty()) m_animationSteps.push_back(step);
    }

    // underserved hosts (reachable but beyond maxHops) all turn orange at once
    if (!state.result.underservedHosts.empty()) {
        std::vector<ColorChange> step;
        for (int id : state.result.underservedHosts)
            step.push_back({ id, {}, COL_SEMICRIT, 2.0f });
        m_animationSteps.push_back(step);
    }

    // unreachable hosts in red
    for (int id : state.result.unreachableHosts)
        m_animationSteps.push_back({{ {id, {}, COL_UNREACHABLE, 2.0f} }});
}

void GraphRenderer::buildBridgeAnimation(const AppState& state) {
    m_animationSteps.clear();
    resetColors(state);

    // critical and semi-critical edges revealed one by one
    for (const auto& [e, crit] : state.result.edgeCriticality)
        if (crit == EdgeCriticality::Critical)
            m_animationSteps.push_back({{ {-1, e, COL_CRITICAL, 3.5f} }});
    for (const auto& [e, crit] : state.result.edgeCriticality)
        if (crit == EdgeCriticality::SemiCritical)
            m_animationSteps.push_back({{ {-1, e, COL_SEMICRIT, 2.5f} }});

    // redundant edges all turn green at once
    std::vector<ColorChange> redundantStep;
    for (const auto& [e, crit] : state.result.edgeCriticality)
        if (crit == EdgeCriticality::Redundant)
            redundantStep.push_back({ -1, e, COL_REDUNDANT, 2.0f });
    if (!redundantStep.empty()) m_animationSteps.push_back(redundantStep);
}

bool GraphRenderer::step(AppState& state) {
    if (state.animationStep >= (int)m_animationSteps.size()) return false;

    for (const ColorChange& c : m_animationSteps[state.animationStep]) {
        if (c.nodeId != -1) {
            for (auto& n : m_nodes)
                if (n.id == c.nodeId) { n.color = c.color; break; }
        } else {
            for (auto& e : m_edges)
                if ((e.edge.from == c.edge.from && e.edge.to == c.edge.to) ||
                    (e.edge.from == c.edge.to   && e.edge.to == c.edge.from)) {
                    e.color = c.color;
                    e.thickness = c.thickness;
                    break;
                }
        }
    }
    ++state.animationStep;
    return state.animationStep < (int)m_animationSteps.size();
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

void GraphRenderer::drawEdge(ImDrawList* dl, const EdgeVisual& ev) const {
    ImVec2 a = nodePos(m_nodes, ev.edge.from);
    ImVec2 b = nodePos(m_nodes, ev.edge.to);
    dl->AddLine(a, b, ev.color, ev.thickness);
}

void GraphRenderer::drawNode(ImDrawList* dl, const NodeVisual& nv, const AppState& state) const {
    ImVec2 p = nv.pos;
    ImU32 col = nv.color;
    NodeType type = state.graph.getNode(nv.id).type;

    if (type == NodeType::Provider) {
        float w = 14, h = 22;
        ImVec2 top    = {p.x - w/2, p.y - h/2};
        ImVec2 bottom = {p.x + w/2, p.y + h/2};
        dl->AddRectFilled(top, bottom, col, 2.0f);
        float antH = 8;
        dl->AddLine({p.x - 4, top.y}, {p.x - 4, top.y - antH}, col, 1.5f);
        dl->AddLine({p.x + 4, top.y}, {p.x + 4, top.y - antH}, col, 1.5f);

    } else if (type == NodeType::Router) {
        float r = 14;
        dl->AddQuadFilled({p.x, p.y-r}, {p.x+r, p.y}, {p.x, p.y+r}, {p.x-r, p.y}, col);
        float t = 5;
        dl->AddLine({p.x,   p.y-r-t}, {p.x,   p.y-r+t}, col, 1.5f);
        dl->AddLine({p.x,   p.y+r-t}, {p.x,   p.y+r+t}, col, 1.5f);
        dl->AddLine({p.x-r-t, p.y},   {p.x-r+t, p.y},   col, 1.5f);
        dl->AddLine({p.x+r-t, p.y},   {p.x+r+t, p.y},   col, 1.5f);

    } else {
        float w = 18, h = 14;
        ImVec2 screenTop    = {p.x - w/2, p.y - h/2};
        ImVec2 screenBottom = {p.x + w/2, p.y + h/2};
        dl->AddRectFilled(screenTop, screenBottom, col, 2.0f);
        float standBottom = p.y + h/2 + 6;
        dl->AddLine({p.x, p.y + h/2}, {p.x, standBottom}, col, 2.0f);
        dl->AddLine({p.x - 5, standBottom}, {p.x + 5, standBottom}, col, 2.0f);
    }

    // label below icon
    const auto& label = state.graph.getNode(nv.id).label;
    dl->AddText({p.x - ImGui::CalcTextSize(label.c_str()).x / 2, p.y + 20}, IM_COL32( 50,  50,  50, 255), label.c_str());
}

void GraphRenderer::draw(ImDrawList* dl, ImVec2 origin, ImVec2 canvasSize, const AppState& state) {
    // offset all positions by canvas origin
    for (auto& n : m_nodes) n.pos = {n.pos.x + origin.x, n.pos.y + origin.y};

    for (const auto& e : m_edges) drawEdge(dl, e);
    for (const auto& n : m_nodes) drawNode(dl, n, state);

    // undo offset so positions stay canvas-relative
    for (auto& n : m_nodes) n.pos = {n.pos.x - origin.x, n.pos.y - origin.y};
}
