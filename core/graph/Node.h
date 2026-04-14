#ifndef NETWORKREACHABILITYANALYSIS_NODE_H
#define NETWORKREACHABILITYANALYSIS_NODE_H

#include <string>
#include "NodeType.h"

struct Node {
    int id;
    std::string label;
    NodeType type;
};

#endif //NETWORKREACHABILITYANALYSIS_NODE_H
