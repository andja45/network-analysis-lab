#ifndef NETWORKANALYSISLAB_GRAPHRENDERER_H
#define NETWORKANALYSISLAB_GRAPHRENDERER_H

#include "AppState.h"
#include "imgui.h"
#include <vector>
#include <unordered_map>

struct NodeVisual {
    int id;
    ImVec2 pos;
    ImU32 color;
};

struct EdgeVisual {
    Edge edge;
    ImU32 color;
    float thickness;
};

struct ColorChange {
    int nodeId = -1;
    Edge edge;
    ImU32 color;
    float thickness = 2.0f;
};

class GraphRenderer {
private:
    std::vector<NodeVisual> m_nodes;
    std::vector<EdgeVisual> m_edges;
    std::unordered_map<int, ImU32> m_providerColors;
    std::vector<std::vector<ColorChange>> m_animationSteps;

    void computeLayout(const AppState& state, ImVec2 canvasSize);
    void resetColors(const AppState& state);
    void drawNode(ImDrawList* dl, const NodeVisual& nv, const AppState& state) const;
    void drawEdge(ImDrawList* dl, const EdgeVisual& ev) const;
public:
    void reset(const AppState& state, ImVec2 canvasSize);
    void draw(ImDrawList* drawList, ImVec2 canvasOrigin, ImVec2 canvasSize, const AppState& state);
    void buildBFSAnimation(const AppState& state);
    void buildBridgeAnimation(const AppState& state);
    bool step(AppState& state); // returns false when the last step was just applied
    void clear(const AppState& state);

    void addNode(int id, ImVec2 pos);
    void addEdge(const Edge& e);
    void removeNode(int id);
    void removeEdge(const Edge& e);
    void moveNode(int id, ImVec2 delta);
    void setNodePosition(int id, ImVec2 pos);

    int    nodeAt(ImVec2 p) const;
    Edge   edgeAt(ImVec2 p) const;
    ImVec2 nodePosition(int id) const; // for animation when creating an edge
};

#endif
