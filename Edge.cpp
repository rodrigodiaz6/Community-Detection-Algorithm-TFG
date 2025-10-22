#include "Edge.h"
#include "Node.h"

namespace networkStructure {
Edge::Edge(unsigned int id0, Node* node1, Node* node2, double weight0) {
    id = id0;
    n1 = node1;
    n2 = node2;
    weight = weight0;
}

unsigned int Edge::getID() {
    return id;
}

Node* Edge::getOrigin() {
    return n1;
}

Node* Edge::getDestiny() {
    return n2;
}

double Edge::getWeight() {
    return weight;
}

Node* Edge::getOpposite(Node *node) {
    if (node->equals(n1))
        return n2;
    else if (node->equals(n2))
        return n1;
    else
        return nullptr;
}

bool Edge::equals(Edge *edge) {
    if (edge == nullptr) return false;
    return (id == edge->getID());
}

} // namespace networkStructure