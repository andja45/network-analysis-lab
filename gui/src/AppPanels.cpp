#include "App.h"
#include "AppTheme.h"
#include "examples/GraphExamples.h"
#include <cstdio>
#include <string>

extern const ImVec2 LAYOUT_CROSSROADS[];
extern const ImVec2 LAYOUT_CITY_RING[];
extern const ImVec2 LAYOUT_DUAL_ISP[];

static bool highlightedButton(const char* name, bool active, ImVec2 size = {-1, 0}) {
    if (active) ImGui::PushStyleColor(ImGuiCol_Button, AppTheme::BTN_SELECTED);
    bool clicked = ImGui::Button(name, size);
    if (active) ImGui::PopStyleColor();
    return clicked;
}

static const char* algoName(AlgoChoice a) { return a == AlgoChoice::Dijkstra ? "Dijkstra" : "A*"; }
static const char* metricName(MetricChoice m) {
    switch (m) {
        case MetricChoice::Fastest: return "Fastest";
        case MetricChoice::Cheapest: return "Cheapest";
        case MetricChoice::LeastLoaded: return "Least Loaded";
        case MetricChoice::MostReliable: return "Most Reliable";
        default: return "Balanced";
    }
}
static const char* dcName(DetourCriticality dc) {
    switch (dc) {
        case DetourCriticality::Bridge: return "bridge";
        case DetourCriticality::Critical: return "critical";
        case DetourCriticality::SemiCritical: return "semi";
        default: return "redundant";
    }
}
static ImVec4 dcColor(DetourCriticality dc) {
    if (dc == DetourCriticality::Bridge || dc == DetourCriticality::Critical) return AppTheme::TEXT_RED;
    if (dc == DetourCriticality::SemiCritical) return AppTheme::TEXT_ORANGE;
    return AppTheme::TEXT_GREEN;
}

void App::leftPanel() {
    ImGui::Text("Node type");
    for (auto [name, type] : {std::pair{"Provider", NodeType::Provider},
                               std::pair{"Router", NodeType::Router},
                               std::pair{"Host", NodeType::Host}}) {
        bool sel = m_state.selectedType == type;
        if (highlightedButton(name, sel, {-1, 0}))
            m_state.selectedType = sel ? std::optional<NodeType>{} : type;
    }
    ImGui::Separator();

    PanelMode prevMode = m_state.panelMode;
    if (ImGui::BeginTabBar("##modes")) {
        if (ImGui::BeginTabItem("Reachability")) {
            if (prevMode != PanelMode::Reachability) { syncClear(); m_state.viewMode = ViewMode::Neutral; }
            m_state.panelMode = PanelMode::Reachability;
            auto runAnim = [&](bool bfs) {
                m_state.runAnalysis();
                bfs ? m_top.buildBFSAnimation(m_state) : m_top.buildBridgeAnimation(m_state);
                m_state.animationState = AnimationState::Running;
                m_state.animationStep = 0;
                m_state.timeSinceStep = 0.0f;
                m_state.viewMode = bfs ? ViewMode::BFS : ViewMode::Bridges;
            };
            if (ImGui::Button("Run BFS", {-1, 0})) runAnim(true);
            if (ImGui::Button("Run Bridge Detection", {-1, 0})) runAnim(false);
            ImGui::SliderInt("Max hops", &m_state.maxHops, -1, 10);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Routing")) {
            if (prevMode != PanelMode::Routing) { syncClear(); m_state.viewMode = ViewMode::Neutral; }
            m_state.panelMode = PanelMode::Routing;
            if (highlightedButton("Split view", m_state.splitView, {-1, 0})) {
                m_state.splitView = !m_state.splitView;
                float sy = m_state.splitView ? 0.5f : 2.0f;
                for (const auto& [id, _] : m_state.graph.nodes()) {
                    ImVec2 pos = m_top.nodePosition(id);
                    syncSetNodePosition(id, {pos.x, pos.y * sy});
                }
            }
            auto pick = [&](const char* name, bool& picking, bool& other) {
                if (highlightedButton(name, picking, {-1, 0})) { other = false; picking = !picking; }
            };
            pick("Source", m_state.pickingSource, m_state.pickingDest);
            pick("Dest", m_state.pickingDest, m_state.pickingSource);
            ImGui::SeparatorText("Next edge weights");
            ImGui::SliderFloat("Latency", &m_state.pendingLatency, 0.1f, 50.0f, "%.1f");
            ImGui::SliderFloat("Price", &m_state.pendingPrice, 0.1f, 50.0f, "%.1f");
            ImGui::SliderFloat("Bandwidth", &m_state.pendingBandwidth, 10.0f, 10000.0f, "%.0f");
            ImGui::SliderFloat("Load", &m_state.pendingLoad, 0.0f, 0.99f, "%.2f");
            ImGui::SliderFloat("Reliability", &m_state.pendingReliability, 0.5f, 1.0f, "%.2f");
            const char* metrics[] = {"Fastest", "Cheapest", "Least Loaded", "Reliable", "Balanced"};
            auto canvasRow = [&](RoutingCanvasState& c, const char* rowLabel, const char* id) {
                ImGui::PushID(id);
                ImGui::Text("%s", rowLabel);
                if (highlightedButton("Dijkstra", c.algo == AlgoChoice::Dijkstra, {0, 0})) c.algo = AlgoChoice::Dijkstra;
                ImGui::SameLine();
                if (highlightedButton("A*", c.algo == AlgoChoice::AStar, {0, 0})) c.algo = AlgoChoice::AStar;
                int idx = (int)c.metric;
                if (ImGui::Combo("##m", &idx, metrics, 5)) c.metric = (MetricChoice)idx;
                ImGui::PopID();
            };
            canvasRow(m_state.topCanvas, "Top", "top");
            canvasRow(m_state.bottomCanvas, "Bottom", "bot");
            bool canRoute = m_state.routingSrc != -1 && m_state.routingDst != -1;
            const char* hint = m_state.routingSrc == -1 ? "Pick source first" : "Pick dest first";
            auto action = [&](const char* name, bool enabled, const char* disabledHint, auto fn) {
                if (!enabled) ImGui::BeginDisabled();
                if (ImGui::Button(name, {-1, 0})) fn();
                if (!enabled) { ImGui::EndDisabled(); ImGui::TextDisabled("%s", disabledHint); }
            };
            action("Run Routing", canRoute, hint, [&] {
                auto h = [&](int u, int v) {
                    ImVec2 pu = m_top.nodePosition(u), pv = m_top.nodePosition(v);
                    return hypotf(pu.x - pv.x, pu.y - pv.y) / 1000.0f;
                };
                m_state.runRouting(h);
                m_top.buildRoutingAnimation(m_state, m_state.topCanvas);
                m_bot.buildRoutingAnimation(m_state, m_state.bottomCanvas);
            });
            action("Run DCI", m_state.topCanvas.result.dijkstra.found, "Run Routing first", [&] {
                m_state.runResilience();
                m_top.buildDCIAnimation(m_state.topCanvas);
                m_bot.buildDCIAnimation(m_state.bottomCanvas);
            });
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::SliderFloat("max DCI", &m_state.maxDCI, 1.0f, 10.0f, "%.1f");
    ImGui::SliderFloat("Speed", &m_state.stepDelay, 0.05f, 1.0f, "%.2f s");
    ImGui::SeparatorText("Examples");
    if (ImGui::Button("Crossroads", {-1, 0})) loadExample(makeCrossroads(), LAYOUT_CROSSROADS, 7);
    if (ImGui::Button("City Ring", {-1, 0})) loadExample(makeCityRing(), LAYOUT_CITY_RING, 11);
    if (ImGui::Button("Dual ISP", {-1, 0})) loadExample(makeDualISP(), LAYOUT_DUAL_ISP, 17);
    ImGui::Separator();
    if (ImGui::Button("Clear", {-1, 0})) {
        m_state = AppState{};
        syncReset();
        m_labelSeq[0] = m_labelSeq[1] = m_labelSeq[2] = 0;
    }
}

void App::canvas() {
    m_canvasSize = ImGui::GetContentRegionAvail();
    bool splitCanvas = m_state.splitView && m_state.panelMode == PanelMode::Routing;
    ImVec2 activeSize = splitCanvas ? ImVec2{m_canvasSize.x, m_canvasSize.y / 2.0f} : m_canvasSize;

    if (splitCanvas) ImGui::BeginChild("##top_canvas", activeSize, false);

    ImVec2 topOrigin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##input", activeSize);
    bool hovered = ImGui::IsItemHovered();
    ImVec2 rel = {ImGui::GetMousePos().x - topOrigin.x, ImGui::GetMousePos().y - topOrigin.y};

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        int hit = m_top.nodeAt(rel);
        bool changed = false;
        if (hit != -1) {
            m_state.graph.removeNode(hit);
            syncRemoveNode(hit);
            if (m_state.pendingEdgeFrom == hit) m_state.pendingEdgeFrom = -1;
            if (m_state.routingSrc == hit) m_state.routingSrc = -1;
            if (m_state.routingDst == hit) m_state.routingDst = -1;
            changed = true;
        } else {
            Edge e = m_top.edgeAt(rel);
            if (e.from != -1) { m_state.graph.removeEdge(e.from, e.to); syncRemoveEdge(e); changed = true; }
        }
        if (changed) {
            m_state.result = ReachabilityResult{};
            m_state.viewMode = ViewMode::Neutral;
            m_state.animationState = AnimationState::Idle;
            syncClear();
        }
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        int hit = m_top.nodeAt(rel);
        if (hit != -1) {
            if (m_state.pickingSource) { m_state.routingSrc = hit; m_state.pickingSource = false; }
            else if (m_state.pickingDest) { m_state.routingDst = hit; m_state.pickingDest = false; }
            else if (m_state.pendingEdgeFrom != -1 && hit != m_state.pendingEdgeFrom) {
                if (m_state.graph.addEdge(m_state.pendingEdgeFrom, hit,
                        m_state.pendingLatency, m_state.pendingPrice,
                        m_state.pendingBandwidth, m_state.pendingLoad, m_state.pendingReliability)) {
                    const Edge* full = m_state.graph.findEdge(m_state.pendingEdgeFrom, hit);
                    if (full) syncAddEdge(*full);
                }
                m_state.pendingEdgeFrom = m_state.draggedNode = -1;
            } else {
                m_state.pendingEdgeFrom = m_state.draggedNode = hit;
            }
        } else if (m_state.selectedType.has_value()) {
            m_state.pendingEdgeFrom = -1;
            static const char pfx[] = {'P', 'R', 'H'};
            int ti = (int)*m_state.selectedType;
            char lbl[8]; std::snprintf(lbl, 8, "%c%d", pfx[ti], ++m_labelSeq[ti]);
            int id = m_state.graph.addNode(lbl, *m_state.selectedType);
            syncAddNode(id, rel);
        }
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && m_state.draggedNode != -1) {
        ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        syncMoveNode(m_state.draggedNode, delta);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        m_state.pendingEdgeFrom = -1;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) m_state.draggedNode = -1;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    bool showLabels = m_state.panelMode == PanelMode::Routing;
    m_top.draw(dl, topOrigin, activeSize, m_state, m_state.topCanvas.metric, showLabels);

    static constexpr ImU32 GOLD = IM_COL32(220, 170, 30, 255);
    auto drawRing = [](ImDrawList* list, ImVec2 pos, ImVec2 orig, ImU32 col) {
        list->AddCircle({pos.x + orig.x, pos.y + orig.y}, 22.0f, col, 0, 2.5f);
    };
    auto drawSrcDst = [&](ImDrawList* list, GraphRenderer& r, ImVec2 orig) {
        if (m_state.routingSrc != -1) drawRing(list, r.nodePosition(m_state.routingSrc), orig, GOLD);
        if (m_state.routingDst != -1) drawRing(list, r.nodePosition(m_state.routingDst), orig, GOLD);
    };
    if (m_state.panelMode == PanelMode::Routing) drawSrcDst(dl, m_top, topOrigin);
    if (m_state.pendingEdgeFrom != -1) {
        ImVec2 p = m_top.nodePosition(m_state.pendingEdgeFrom);
        dl->AddCircle({p.x + topOrigin.x, p.y + topOrigin.y}, 22.0f, AppTheme::PENDING_EDGE, 0, 2.0f);
    }
    if (m_state.topCanvas.packetPhase) m_top.drawPacket(dl, topOrigin, m_state.topCanvas);

    if (hovered && m_state.draggedNode == -1 && m_state.panelMode == PanelMode::Routing) {
        Edge hit = m_top.edgeAt(rel);
        if (hit.from != -1 && m_top.nodeAt(rel) == -1) {
            const Edge* e = m_state.graph.findEdge(hit.from, hit.to);
            if (e) {
                ImGui::BeginTooltip();
                ImGui::Text("Latency:     %.1f ms", e->latency);
                ImGui::Text("Price:       %.1f", e->price);
                ImGui::Text("Bandwidth:   %.0f Mbps", e->bandwidth);
                ImGui::Text("Load:        %.2f", e->load);
                ImGui::Text("Reliability: %.2f", e->reliability);
                ImGui::EndTooltip();
            }
        }
    }

    if (splitCanvas) {
        ImGui::EndChild();
        ImVec2 botOrigin = ImGui::GetCursorScreenPos();
        ImGui::BeginChild("##bot_canvas", activeSize, false);
        ImDrawList* bdl = ImGui::GetWindowDrawList();
        m_bot.draw(bdl, botOrigin, activeSize, m_state, m_state.bottomCanvas.metric, showLabels);
        drawSrcDst(bdl, m_bot, botOrigin);
        if (m_state.bottomCanvas.packetPhase) m_bot.drawPacket(bdl, botOrigin, m_state.bottomCanvas);
        ImGui::EndChild();
    }
}

void App::rightPanel() {
    auto label = [&](int id) { return m_state.graph.getNode(id).label.c_str(); };

    if (m_state.viewMode == ViewMode::Neutral) {
        if (m_state.panelMode == PanelMode::Routing)
            ImGui::TextDisabled("Pick source and dest,\nthen Run Routing or DCI \nto see results here.");
        else
            ImGui::TextDisabled("Run BFS or Bridge\nDetection to see\nresults here.");
        return;
    }
    const auto& r = m_state.result;
    if (m_state.viewMode == ViewMode::BFS) {
        ImGui::Text("BFS Results"); ImGui::Separator();
        ImGui::Text("Total: %d", r.totalHosts);
        ImGui::Text("Reachable: %d", r.reachableHosts);
        ImGui::TextColored(AppTheme::TEXT_RED, "Unreachable: %d", (int)r.unreachableHosts.size());
        if (m_state.maxHops != -1)
            ImGui::TextColored(AppTheme::TEXT_ORANGE, "Underserved: %d", (int)r.underservedHosts.size());
        ImGui::Separator();
        for (int id : r.unreachableHosts) ImGui::TextColored(AppTheme::TEXT_RED, "  %s", label(id));
        for (int id : r.underservedHosts) ImGui::TextColored(AppTheme::TEXT_ORANGE, "  %s", label(id));
        return;
    }
    if (m_state.viewMode == ViewMode::Bridges) {
        int nCrit = 0, nSemi = 0;
        for (const auto& [e, c] : r.connectionCriticality) {
            if (c == ConnectionCriticality::Critical) ++nCrit;
            else if (c == ConnectionCriticality::SemiCritical) ++nSemi;
        }
        ImGui::Text("Bridge Results"); ImGui::Separator();
        ImGui::TextColored(AppTheme::TEXT_RED, "Critical: %d", nCrit);
        ImGui::TextColored(AppTheme::TEXT_ORANGE, "Semi-critical: %d", nSemi);
        ImGui::TextColored(AppTheme::TEXT_GREEN, "Redundant: %d", (int)r.connectionCriticality.size() - nCrit - nSemi);
        ImGui::Separator();
        for (const Edge& b : r.bridgeResult.bridges) {
            bool crit = r.connectionCriticality.at(b) == ConnectionCriticality::Critical;
            ImGui::TextColored(crit ? AppTheme::TEXT_RED : AppTheme::TEXT_ORANGE,
                "  %s--%s [%s]", label(b.from), label(b.to), crit ? "CRIT" : "SEMI");
        }
        return;
    }

    float panelH = ImGui::GetContentRegionAvail().y / (m_state.splitView ? 2.0f : 1.0f);
    auto drawCanvasStats = [&](const RoutingCanvasState& c, const char* childId) {
        ImGui::BeginChild(childId, {-1, panelH}, false);
        ImGui::Text("%s · %s", algoName(c.algo), metricName(c.metric));
        ImGui::Separator();
        const auto& res = (c.algo == AlgoChoice::Dijkstra) ? c.result.dijkstra : c.result.astar;
        if (!res.found) { ImGui::TextDisabled("No path found"); ImGui::EndChild(); return; }
        std::string pathStr;
        for (int id : res.path) pathStr += (pathStr.empty() ? "" : " -> ") + m_state.graph.getNode(id).label;
        ImGui::TextWrapped("%s", pathStr.c_str());
        ImGui::Text("Cost:        %.2f", res.totalCost);
        ImGui::Text("Relaxations: %d", res.relaxations);
        ImGui::Text("Visited:     %d", res.visited);
        if (!c.dciResult.detourCriticality.empty()) {
            ImGui::SeparatorText("DCI");
            for (const auto& [e, dc] : c.dciResult.detourCriticality) {
                float ratio = c.dciResult.dci.at(e);
                char buf[16]; std::snprintf(buf, 16, std::isinf(ratio) ? "inf" : "%.2f", ratio);
                ImGui::TextColored(dcColor(dc), "  %s--%s  %s  %s", label(e.from), label(e.to), buf, dcName(dc));
            }
        }
        ImGui::EndChild();
    };
    drawCanvasStats(m_state.topCanvas, "##stats_top");
    if (m_state.splitView) drawCanvasStats(m_state.bottomCanvas, "##stats_bot");
}
