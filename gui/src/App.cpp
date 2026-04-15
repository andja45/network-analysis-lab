#include "App.h"
#include "AppTheme.h"
#include "examples/GraphExamples.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

extern const ImVec2 LAYOUT_CROSSROADS[] = {
    {430, 80}, {430, 200}, {210, 340}, {430, 340},
    {650, 340}, {430, 480}, {590, 480},
};
extern const ImVec2 LAYOUT_CITY_RING[] = {
    {60, 280}, {740, 280}, {230, 280}, {315, 133},
    {485, 133}, {570, 280}, {485, 427}, {315, 427},
    {450, 540}, {560, 540}, {485, 50},
};
extern const ImVec2 LAYOUT_DUAL_ISP[] = {
    {30, 280}, {790, 260}, {120, 200}, {240, 200},
    {240, 360}, {120, 360}, {550, 260}, {690, 190},
    {690, 340}, {100, 460}, {260, 460}, {690, 440},
    {80, 550}, {260, 550}, {650, 550}, {740, 550}, {430, 550},
};

App::App() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    m_window = glfwCreateWindow(1280, 720, "Network Analysis Lab", nullptr, nullptr);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGui::StyleColorsLight();
    auto& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = AppTheme::BG_WINDOW;
    style.Colors[ImGuiCol_ChildBg] = AppTheme::BG_PANEL;
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    loadExample(makeCrossroads(), LAYOUT_CROSSROADS, 7);
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

        float dt = ImGui::GetIO().DeltaTime;

        if (m_state.animationState == AnimationState::Running) {
            m_state.timeSinceStep += dt;
            if (m_state.timeSinceStep >= m_state.stepDelay) {
                m_state.timeSinceStep = 0.0f;
                if (!m_top.step(m_state.animationStep))
                    m_state.animationState = AnimationState::Done;
            }
        }

        if (m_state.panelMode == PanelMode::Routing) {
            auto tick = [&](GraphRenderer& r, RoutingCanvasState& c) {
                if (c.animationState == AnimationState::Running && !r.step(c.animationStep)) {
                    c.animationState = AnimationState::Done;
                    c.packetPhase = true;
                }
            };
            bool either = m_state.topCanvas.animationState == AnimationState::Running
                       || m_state.bottomCanvas.animationState == AnimationState::Running;
            if (either) {
                m_state.timeSinceStep += dt;
                if (m_state.timeSinceStep >= m_state.stepDelay) {
                    m_state.timeSinceStep = 0.0f;
                    tick(m_top, m_state.topCanvas);
                    tick(m_bot, m_state.bottomCanvas);
                }
            }
            if (m_state.topCanvas.packetPhase) m_top.advancePacket(m_state.topCanvas, dt, 3.0f);
            if (m_state.bottomCanvas.packetPhase) m_bot.advancePacket(m_state.bottomCanvas, dt, 3.0f);
        }

        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("##root", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::BeginChild("##left", {200, 0}, true); leftPanel(); ImGui::EndChild(); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, AppTheme::BG_CANVAS);
        ImGui::BeginChild("##canvas", {-220, 0}); canvas(); ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::BeginChild("##right", {220, 0}, true); rightPanel(); ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        int w, h; glfwGetFramebufferSize(m_window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(AppTheme::BG_WINDOW.x, AppTheme::BG_WINDOW.y, AppTheme::BG_WINDOW.z, AppTheme::BG_WINDOW.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }
}

void App::syncAddNode(int id, ImVec2 pos) { m_top.addNode(id, pos); m_bot.addNode(id, pos); }
void App::syncRemoveNode(int id) { m_top.removeNode(id); m_bot.removeNode(id); }
void App::syncAddEdge(const Edge& e) { m_top.addEdge(e); m_bot.addEdge(e); }
void App::syncRemoveEdge(const Edge& e) { m_top.removeEdge(e); m_bot.removeEdge(e); }
void App::syncMoveNode(int id, ImVec2 delta) { m_top.moveNode(id, delta); m_bot.moveNode(id, delta); }
void App::syncSetNodePosition(int id, ImVec2 pos) { m_top.setNodePosition(id, pos); m_bot.setNodePosition(id, pos); }
void App::syncClear() { m_top.clear(); m_bot.clear(); }

void App::syncReset() {
    m_top.reset(m_state);
    m_bot.reset(m_state);
}

void App::loadExample(const Graph& g, const ImVec2* positions, int count) {
    PanelMode mode = m_state.panelMode;
    bool split = m_state.splitView;
    m_state = AppState{};
    m_state.graph = g;
    m_state.panelMode = mode;
    m_state.splitView = split;
    m_state.runAnalysis();
    if (m_canvasSize.x < 1) m_canvasSize = {860, 680};
    syncReset();
    bool splitting = split && mode == PanelMode::Routing;
    float sx = m_canvasSize.x / 860.0f;
    float sy = (splitting ? m_canvasSize.y / 2.0f : m_canvasSize.y) / 680.0f;
    for (int i = 0; i < count; ++i)
        syncSetNodePosition(i, {positions[i].x * sx, positions[i].y * sy});
}
