#include "Network.h"
#include <algorithm>
#include <vector>

namespace networkStructure {

std::size_t Network::getNNodes(){
    return nodes.size();
}

std::size_t Network::getNEdges(){
    return edges.size();
}

Node* Network::getNode(unsigned int id){
    auto it = nodes.find(id);
    return (it == nodes.end()) ? nullptr : it->second.get();
}

Edge* Network::getEdge(unsigned int id){
    auto it = edges.find(id);
    return (it == edges.end()) ? nullptr : it->second.get();
}

Node* Network::addNode(unsigned int id) {
    auto it = nodes.find(id);
    // Si el nodo ya existe, devolvemos el puntero existente.
    if (it != nodes.end()) {
        return it->second.get();
    }
    // Si no, creamos uno nuevo, lo movemos al mapa y devolvemos el puntero.
    auto ptr = std::make_unique<Node>(id);
    Node* raw_ptr = ptr.get();
    nodes.emplace(id, std::move(ptr));
    return raw_ptr;
}

Edge* Network::addEdge(unsigned int id_origin, unsigned int id_destiny, double weight) {
    // Usamos addNode para asegurar que los nodos existan.
    Node* n1 = addNode(id_origin);
    Node* n2 = addNode(id_destiny);

    // Creamos la nueva arista con un ID único.
    unsigned int eid = next_edge_id++;
    auto eptr = std::make_unique<Edge>(eid, n1, n2, weight);
    Edge* raw_ptr = eptr.get();

    // Registramos la arista en la red.
    edges.emplace(eid, std::move(eptr));

    // Añadimos la arista a la lista de adyacencia de los nodos.
    n1->addEdge(raw_ptr);
    // Si no es un bucle, la añadimos también al nodo destino.
    if (n1 != n2) {
        n2->addEdge(raw_ptr);
    }
    return raw_ptr;
}

void Network::removeEdge(unsigned int id){
    auto it = edges.find(id);
    if (it == edges.end()) return; // La arista no existe.

    Edge* e = it->second.get();
    Node* o = e->getOrigin();
    Node* d = e->getDestiny();

    // Eliminar la arista de las listas de adyacencia de los nodos conectados.
    if (o) o->eraseEdge(e);
    if (d && d != o) d->eraseEdge(e);

    // Eliminar la arista del mapa de la red.
    edges.erase(it);
}

void Network::removeNode(unsigned int id){
    auto it = nodes.find(id);
    if (it == nodes.end()) return; // El nodo no existe.

    Node* n = it->second.get();
    
    // Es necesario copiar la lista de aristas antes de iterar, porque `removeEdge` modificará la lista de adyacencia del nodo.
    std::vector<Edge*> incident_edges = n->getAdjList();
    for (Edge* e : incident_edges) {
        //Eliminar la arista de la red.
        if (e) removeEdge(e->getID());
    }

    // Finalmente, eliminar el nodo del mapa de la red.
    nodes.erase(it);
}

std::vector<Edge*> Network::getEdgesOfNode(unsigned int id){
    Node* n = getNode(id);
    if (!n) return {}; // Devuelve un vector vacío si el nodo no existe.
    return n->getAdjList();
}

} // namespace networkStructure