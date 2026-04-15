#include "App.h"
#include "examples/GraphExamples.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>

static constexpr ImVec4 RED = {0.86f, 0.23f, 0.23f, 1.0f};
static constexpr ImVec4 ORANGE = {0.90f, 0.55f, 0.15f, 1.0f};
static constexpr ImVec4 GREEN = {0.23f, 0.70f, 0.31f, 1.0f};

App::App() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    m_window = glfwCreateWindow(1280, 720,"Network Reachability Analysis", nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    auto& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]= {0.95f, 0.95f, 0.95f, 1.0f};
    style.Colors[ImGuiCol_ChildBg]= {0.92f, 0.92f, 0.92f, 1.0f};
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    loadExample(makeOfficeNetwork());
}

App::~App() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void App::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto& io = ImGui::GetIO();

        // advance animation by one step every stepDelay seconds
        if (m_state.animationState == AnimationState::Running) {
            m_state.timeSinceStep += io.DeltaTime;
            if (m_state.timeSinceStep >= m_state.stepDelay) {
                m_state.timeSinceStep = 0.0f;
                if (!m_renderer.step(m_state))
                    m_state.animationState = AnimationState::Done;
            }
        }

        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##root",nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::BeginChild("##left",   {200, 0}, true);  leftPanel();  ImGui::EndChild(); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
        ImGui::BeginChild("##canvas", {-220, 0}); canvas(); ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::BeginChild("##right",  {220, 0}, true); rightPanel(); ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        int w, h; glfwGetFramebufferSize(m_window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }
}

void App::leftPanel() {
    // toggle buttons: click to select type, click again to deselect
    ImGui::Text("Node type");
    auto typeButton = [&](const char* name, NodeType type) {
        bool selected = m_state.selectedType == type;
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.15f, 0.35f, 0.65f, 1.0f});
        if (ImGui::Button(name, {-1, 0}))
            m_state.selectedType = selected ? std::optional<NodeType>{} : type;
        if (selected) ImGui::PopStyleColor();
    };
    typeButton("Provider", NodeType::Provider);
    typeButton("Router", NodeType::Router);
    typeButton("Host", NodeType::Host);
    ImGui::Separator();

    auto runAnim = [&](bool bfs) {
        m_state.runAnalysis();
        bfs ? m_renderer.buildBFSAnimation(m_state) : m_renderer.buildBridgeAnimation(m_state);
        m_state.animationState = AnimationState::Running;
        m_state.animationStep = 0;
        m_state.timeSinceStep = 0.0f;
        m_state.viewMode = bfs ? ViewMode::BFS : ViewMode::Bridges;
    };
    if (ImGui::Button("Run BFS",{-1, 0})) runAnim(true);
    if (ImGui::Button("Run Bridge Detection", {-1, 0})) runAnim(false);
    ImGui::Separator();

    ImGui::SliderInt("BFS Max hops", &m_state.maxHops,-1,10);
    ImGui::SliderFloat("Speed", &m_state.stepDelay, 0.05f,1.0f, "%.2f s");
    ImGui::Separator();

    ImGui::Text("Example Networks");
    if (ImGui::Button("Office",{-1, 0})) loadExample(makeOfficeNetwork());
    if (ImGui::Button("Redundant",{-1, 0})) loadExample(makeRedundantNetwork());
    if (ImGui::Button("Linear",{-1, 0})) loadExample(makeLinearNetwork());
    ImGui::Separator();

    if (ImGui::Button("Clear", {-1, 0})) {
        m_state = AppState{};
        m_renderer.reset(m_state, m_canvasSize);
        m_labelSeq[0] = m_labelSeq[1] = m_labelSeq[2] = 0;
    }
}

void App::canvas() {
    ImVec2 origin = ImGui::GetCursorScreenPos();
    m_canvasSize = ImGui::GetContentRegionAvail();

    // invisible button to capture mouse events
    ImGui::InvisibleButton("##input", m_canvasSize);
    bool   hovered = ImGui::IsItemHovered();
    ImVec2 mouse = ImGui::GetMousePos();
    ImVec2 rel = { mouse.x - origin.x, mouse.y - origin.y };

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        int hit = m_renderer.nodeAt(rel);
        bool changed = false;
        if (hit != -1) { // delete node
            m_state.graph.removeNode(hit);
            m_renderer.removeNode(hit);
            if (m_state.pendingEdgeFrom == hit) m_state.pendingEdgeFrom = -1;
            changed = true;
        } else { // delete edge
            Edge e = m_renderer.edgeAt(rel);
            if (e.from != -1) { m_state.graph.removeEdge(e.from, e.to); m_renderer.removeEdge(e); changed = true; }
        }
        if (changed) {
            m_state.result = ReachabilityResult{};
            m_state.viewMode = ViewMode::Neutral;
            m_state.animationState = AnimationState::Idle;
            m_renderer.clear(m_state);
        }
    }

    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        int hit = m_renderer.nodeAt(rel);
        if (hit != -1 && m_state.pendingEdgeFrom != -1 && hit != m_state.pendingEdgeFrom) { // add edge
            if (m_state.graph.addEdge(m_state.pendingEdgeFrom, hit))
                m_renderer.addEdge({m_state.pendingEdgeFrom, hit});
            m_state.pendingEdgeFrom = m_state.draggedNode = -1;
        } else if (hit != -1) { // start dragging existing node
            m_state.pendingEdgeFrom = m_state.draggedNode = hit;
        } else if (m_state.selectedType.has_value()) { // add new node if a type is selected
            m_state.pendingEdgeFrom = -1;
            static const char pfx[] = {'P', 'R', 'H'};
            int ti = (int)*m_state.selectedType;
            char label[8]; std::snprintf(label, 8, "%c%d", pfx[ti], ++m_labelSeq[ti]);
            int id = m_state.graph.addNode(label, *m_state.selectedType);
            m_renderer.addNode(id, rel);
        }
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && m_state.draggedNode != -1) {
        m_renderer.moveNode(m_state.draggedNode, ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        m_state.pendingEdgeFrom = -1;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) m_state.draggedNode = -1;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    m_renderer.draw(dl, origin, m_canvasSize, m_state);

    if (m_state.pendingEdgeFrom != -1) {
        ImVec2 p = m_renderer.nodePosition(m_state.pendingEdgeFrom);
        dl->AddCircle({p.x + origin.x, p.y + origin.y}, 22.0f, IM_COL32(255, 255, 100, 200), 0, 2.0f);
    }
}

void App::rightPanel() {
    if (m_state.viewMode == ViewMode::Neutral) {
        ImGui::TextDisabled("Run BFS or Bridge\nDetection to see\nresults here.");
        return;
    }

    const auto& r = m_state.result;
    auto label = [&](int id) { return m_state.graph.getNode(id).label.c_str(); };

    if (m_state.viewMode == ViewMode::BFS) {
        ImGui::Text("BFS Results");  ImGui::Separator();
        ImGui::Text("Total: %d", r.totalHosts);
        ImGui::Text("Reachable: %d", r.reachableHosts);
        ImGui::TextColored(RED,    "Unreachable: %d", (int)r.unreachableHosts.size());
        if (m_state.maxHops != -1) // only when max hop is active
            ImGui::TextColored(ORANGE, "Underserved: %d", (int)r.underservedHosts.size());
        ImGui::Separator();
        for (int id : r.unreachableHosts)
            ImGui::TextColored(RED,"  %s", label(id));
        for (int id : r.underservedHosts)
            ImGui::TextColored(ORANGE,"  %s", label(id));
        return;
    }

    // count edges by criticality
    int nCrit = 0, nSemi = 0, nRedund = 0;
    for (const auto& [e, c] : r.connectionCriticality) {
        if (c == ConnectionCriticality::Critical) ++nCrit;
        else if (c == ConnectionCriticality::SemiCritical) ++nSemi;
        else ++nRedund;
    }

    ImGui::Text("Bridge Results - Connections"); ImGui::Separator();
    ImGui::TextColored(RED,"Critical: %d", nCrit);
    ImGui::TextColored(ORANGE,"Semi-critical: %d", nSemi);
    ImGui::TextColored(GREEN,"Redundant: %d", nRedund);
    ImGui::Separator();

    // list only critical and semi-critical connections
    for (const Edge& b : r.bridgeResult.bridges) {
        auto crit = r.connectionCriticality.at(b);
        bool isCrit = (crit == ConnectionCriticality::Critical);
        ImGui::TextColored(isCrit ? RED : ORANGE, "  %s--%s [%s]",
            label(b.from), label(b.to), isCrit ? "CRIT" : "SEMI");
    }
}

void App::loadExample(const Graph& g) {
    m_state = AppState{};
    m_state.graph = g;
    m_state.runAnalysis();
    if (m_canvasSize.x < 1) m_canvasSize = {860, 680};
    m_renderer.reset(m_state, m_canvasSize);
}
