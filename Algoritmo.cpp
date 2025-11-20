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
    : network(net) {
}

void Algoritmo::initializeCommunities() {
    if (!network) return; // Seguridad por si network es un puntero nulo
    for (auto& pair : network->getNodesMap()) {
        networkStructure::Node* node = pair.second.get();
        node->setCommunity(node->getID());// Cada nodo en su propia comunidad
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

void Algoritmo::run(double min_gain, double gamma) {
    if (!network || network->getNNodes() == 0) {
        return;
    }
    initializeCommunities();
    std::map<int, unsigned int> community_sizes;
    for (auto& pair : network->getNodesMap()) {
        Node* node = pair.second.get();
        if (!node) continue;
        int comm = node->getCommunity();
        community_sizes[comm] += 1;
    }

    if (community_sizes.empty()) {
        return;
    }

    bool communities_changed;
    do {
        communities_changed = false;

        // Obtenemos todos los nodos para procesar y los barajamos
        std::vector<Node*> nodes_to_process;
        for (auto& pair : network->getNodesMap()) {
            nodes_to_process.push_back(pair.second.get());
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(nodes_to_process.begin(), nodes_to_process.end(), g);

        // Bucle principal sobre todos los nodos
        for (Node* currentNode : nodes_to_process) {
            if (!currentNode) continue;

            int current_comm = currentNode->getCommunity();
            unsigned int size_i = 0;
            auto it_size_i = community_sizes.find(current_comm);
            if (it_size_i != community_sizes.end()) {
                size_i = it_size_i->second;
            }
            // Pesos de las aristas del nodo hacia cada comunidad vecina (k_i_in)
            std::map<int, double> neighbor_comm_weights = getNeighborCommunityWeights(currentNode);
            // Peso hacia su propia comunidad
            double k_i_in_i = 0.0;
            auto it_self = neighbor_comm_weights.find(current_comm);
            if (it_self != neighbor_comm_weights.end()) {
                k_i_in_i = it_self->second;
            }

            int best_community = current_comm;
            double max_gain = 0.0;

            // Recorremos comunidades vecinas j
            for (const auto& entry : neighbor_comm_weights) {
                int j = entry.first;
                double k_i_in_j = entry.second;

                if (j == current_comm) {
                    continue; // no tiene sentido "moverse" a la misma comunidad
                }

                unsigned int size_j = 0;
                auto it_size_j = community_sizes.find(j);
                if (it_size_j != community_sizes.end()) {
                    size_j = it_size_j->second;
                }

                // ΔQ_CPM = (k_i^B - k_i^A) + γ · (n_A − n_B − 1)
                double delta = (k_i_in_j - k_i_in_i) + gamma * (static_cast<double>(size_i) - static_cast<double>(size_j) - 1.0);
                if (delta - max_gain > min_gain) {
                    max_gain = delta;
                    best_community = j;
                }
            }

            // Si hemos encontrado una comunidad mejor según CPM, movemos el nodo
            if (best_community != current_comm) {
                currentNode->setCommunity(best_community);
                communities_changed = true;

                // Actualizamos tamaños de comunidades
                if (community_sizes[current_comm] > 0) {
                    community_sizes[current_comm] -= 1;
                }
                community_sizes[best_community] += 1;
            }
        }
    } while (communities_changed);
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
