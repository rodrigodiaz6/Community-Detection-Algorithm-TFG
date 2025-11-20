#include "Algoritmo.h"
#include <vector>
#include <map>
#include <algorithm> 
#include <random>    
#include <numeric>   
#include <iostream>  
#include <unordered_map>
#include <unordered_set>

namespace networkStructure {

Algoritmo::Algoritmo(networkStructure::Network* net)
    : network(net), m(0.0) {
}

void Algoritmo::initializeCommunities() {
    if (!network) return; // Seguridad por si network es un puntero nulo
    for (auto& pair : network->getNodesMap()) {
        networkStructure::Node* node = pair.second.get();
        node->setCommunity(node->getID());// Cada nodo en su propia comunidad
    }
}

void Algoritmo::precomputeAggregates() {
    m = 0.0;
    node_degrees.clear();
    community_degrees.clear();

    if (!network) return;

    for (auto& pair : network->getEdgesMap()) {
        m += pair.second->getWeight();
    }
    
    if (m == 0.0) {
        return;
    }

    for (auto& pair : network->getNodesMap()) {
        networkStructure::Node* node = pair.second.get();
        unsigned int node_id = node->getID();
        int node_comm = node->getCommunity();
        double k_i = 0.0; 
        for (networkStructure::Edge* edge : node->getAdjList()) {
            k_i += edge->getWeight();// Grado ponderado del nodo
        }
        node_degrees[node_id] = k_i;// Almacenamos k_i
        community_degrees[node_comm] += k_i;// Grado total de la comunidad
    }
}

std::map<int, double> Algoritmo::getNeighborCommunityWeights(networkStructure::Node* node) {
    std::map<int, double> weights;
    
    for (networkStructure::Edge* edge : node->getAdjList()) {// Recorremos aristas incidentes
        networkStructure::Node* neighbor = edge->getOpposite(node);// Nodo vecino
        int neighbor_comm = neighbor->getCommunity();// Comunidad del vecino
        weights[neighbor_comm] += edge->getWeight();// Suma de pesos a la comunidad del vecino
    }
    return weights;
}

void Algoritmo::run(double min_modularity_gain) {
    if (!network || network->getNNodes() == 0) {
        return; 
    }
    // Inicializamos cada nodo en su propia comunidad.
    initializeCommunities();

    // Precalculamos los datos agregados necesarios.
    // m y node_degrees (k_i) son constantes.
    // community_degrees (Sigma_tot) se actualizará de manera incremental.
    precomputeAggregates();

    if (m == 0.0) {
        return;
    }
    bool communities_changed;
    do {
        communities_changed = false; // Reiniciamos el indicador de cambios

        // Obtenemos todos los nodos para procesar
        std::vector<networkStructure::Node*> nodes_to_process;
        for (auto& pair : network->getNodesMap()) {
            nodes_to_process.push_back(pair.second.get());
        }

        // Iteramos en orden aleatorio
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(nodes_to_process.begin(), nodes_to_process.end(), g);

        // Bucle principal sobre todos los nodos
        for (networkStructure::Node* currentNode : nodes_to_process) {
            int i = currentNode->getCommunity(); // Comunidad actual del nodo
            double k_i = node_degrees.at(currentNode->getID()); // k_i es constante

            // Encontrar la mejor comunidad para mover el nodo
            int best_community = i; // La mejor comunidad, inicialmente es la actual
            double max_gain = 0.0;  // Ganancia de quedarse en la comunidad actual

            // Obtenemos los pesos a las comunidades vecinas (k_i_in)
            // Esto se recalcula cada vez, ya que los vecinos pueden haberse movido.
            std::map<int, double> neighbor_comm_weights = getNeighborCommunityWeights(currentNode);

            // Datos de la comunidad actual 'i'
            double k_i_in_i = neighbor_comm_weights[i]; // Peso de 'currentNode' a su comunidad actual
            double sigma_tot_i = community_degrees.at(i); // Sigma_tot_i (grado total de su comunidad actual)
            
            // Iteramos sobre las comunidades vecinas (j)
            for (auto const& [j, k_i_in_j] : neighbor_comm_weights) { // k_i_in_j = Peso de 'currentNode' a la comunidad 'j'
                if (i == j) {
                    continue; // Solo consideramos mover a una comunidad diferente
                }
                // Datos de la comunidad objetivo 'j'
                double sigma_tot_j = community_degrees.count(j) ? community_degrees.at(j) : 0.0;// Usamos .count() por si 'j' no existe
                double D = ((k_i_in_j - k_i_in_i) / m) + ((k_i * (sigma_tot_i - sigma_tot_j - k_i)) / (2*m*m));

                // Verificamos si esta es la mejor ganancia hasta ahora
                if (D - max_gain > min_modularity_gain) {
                    max_gain = D;
                    best_community = j;
                }
            }

            // Si encontramos una comunidad mejor, movemos el nodo
            if (best_community != i) {
                currentNode->setCommunity(best_community);
                communities_changed = true; // Marcamos el cambio

                // Actualizamos los datos agregados(Sigma_tot) para las comunidades afectadas
                community_degrees[i] -= k_i;
                community_degrees[best_community] += k_i;
            }
        }
    } while (communities_changed); // Repetimos hasta que no haya cambios en una pasada completa
}

void Algoritmo::mergeCommunities() {
    if (!network || network->getNNodes() == 0) {
        return;
    }

    // Agrupamos los nodos por su comunidad
    std::map<int, std::vector<Node*>> communities;
    for (const auto &pair : network->getNodesMap()) {
        Node* node = pair.second.get();
        if (!node) continue;
        int comm_id = node->getCommunity();
        communities[comm_id].push_back(node);
    }

    // Calculamos el ID máximo actual para crear nodos con IDs únicos
    unsigned int max_node_id = 0;
    for (const auto &pair : network->getNodesMap()) {
        max_node_id = std::max(max_node_id, pair.first);
    }
    unsigned int next_new_id = max_node_id + 1;

    for (auto &entry : communities) { // Procesamos cada comunidad
        int comm_id = entry.first;
        std::vector<Node*> &comm_nodes = entry.second;
        if (comm_nodes.size() <= 1) {
            continue;
        }
        std::unordered_set<Node*> communitySet(comm_nodes.begin(), comm_nodes.end());
        Node* n_merge = network->addNode(next_new_id++); // Nuevo ID siguiente
        n_merge->setCommunity(comm_id);

        // Agregamos los miembros originales al nuevo nodo
        for (Node* node_i : comm_nodes) {
            if (!node_i) continue;

            const auto &miembros_i = node_i->getMembers();
            if (!miembros_i.empty()) {
                // Si node_i ya era un supernodo, heredamos todos sus miembros
                for (unsigned int mid : miembros_i) {
                    n_merge->addMember(mid);
                }
            } else {
                // Si no tuviera members, añadimos su propio ID
                n_merge->addMember(node_i->getID());
            }
        }
        std::unordered_map<Node*, double> externalWeights; // Mapa de pesos externos: vecino -> peso total acumulado
        // Recorremos las aristas de los nodos de la comunidad
        for (Node* node_i : comm_nodes) {
            if (!node_i) continue;

            const auto &adjList = node_i->getAdjList();
            for (Edge* adjEdge : adjList) {
                if (!adjEdge) continue;

                Node* neighbor = adjEdge->getOpposite(node_i);
                if (!neighbor) continue;
                // Si el vecino está en la misma comunidad, lo ignoramos
                if (communitySet.find(neighbor) != communitySet.end()) {
                    continue;
                }
                // Acumular peso hacia ese vecino externo
                externalWeights[neighbor] += adjEdge->getWeight();
            }
        }
        // Crear aristas (n_merge, neighbor, total_w) en la red
        for (auto &nw : externalWeights) {
            Node* neighbor = nw.first;
            double total_w = nw.second;
            if (!neighbor) continue;

            network->addEdge(n_merge->getID(), neighbor->getID(), total_w);
        }
        // Eliminamos los nodos originales de la comunidad
        for (Node* node_i : comm_nodes) {
            if (!node_i) continue;
            unsigned int old_id = node_i->getID();
            network->removeNode(old_id);
        }
    }
}
} // namespace networkStructure
