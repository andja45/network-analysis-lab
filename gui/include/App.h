#ifndef NETWORKREACHABILITYANALYSIS_APP_H
#define NETWORKREACHABILITYANALYSIS_APP_H

#include "AppState.h"
#include "GraphRenderer.h"
#include "GLFW/glfw3.h"

class App {
private:
    GLFWwindow*   m_window = nullptr;
    AppState      m_state;
    GraphRenderer m_renderer;
    ImVec2        m_canvasSize = {0, 0};
    int           m_labelSeq[3] = {};    // for generic labels, {P,R,H} + number

    void leftPanel();
    void canvas();
    void rightPanel();
    void loadExample(const Graph& g);
public:
    App();
    ~App();
    void run();
};

#endif
