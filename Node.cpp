#include "Node.h"
#include "Edge.h"
#include <algorithm> 
#include <vector>

namespace networkStructure {

Node::Node(unsigned int id0)
    : id(id0), community(1), adjList(), members() {}

unsigned int Node::getID() {
    return id;
}

int Node::getCommunity() {
    return community;
}

void Node::setCommunity(int c) {
    community = c;
}

std::size_t Node::getDegree(){
    return adjList.size(); 
}

const std::vector<Edge*>& Node::getAdjList() {
    return adjList;
}

bool Node::equals(Node *node) {
    if (node == nullptr) return false;
    return (id == node->getID());
}

void Node::addEdge(Edge *edge) {
    if (edge->getDestiny()->equals(this) || edge->getOrigin()->equals(this)) {
        adjList.push_back(edge);
    }
}

void Node::eraseEdge(Edge *edge) {
    auto it = std::remove(adjList.begin(), adjList.end(), edge);
    adjList.erase(it, adjList.end());
}

void Node::eraseAllEdges() {
    adjList.clear();
}

void Node::addMember(unsigned int member_id) {
    members.push_back(member_id);
}

const std::vector<unsigned int>& Node::getMembers() const {
    return members;
}

void Node::setMembers(const std::vector<unsigned int>& new_members) {
    members = new_members;
}

Node::~Node() {
    adjList.clear();
}

} // namespace networkStructure