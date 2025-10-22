#include "Algoritmo.h"
#include <vector>
#include <map>
#include <algorithm> 
#include <random>    
#include <numeric>   
#include <iostream>  

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

double Algoritmo::computeModularityGain(double k_i, double sigma_tot_C, double sigma_tot_D,
                                            double k_i_in_C, double k_i_in_D) {
    double term1 = (k_i_in_D - k_i_in_C) / m; // Primer término de la fórmula
    double term2 = (k_i * (sigma_tot_C - sigma_tot_D - k_i)) / (2.0 * m * m); // Segundo término de la fórmula
    return term1 + term2; // Ganancia total de modularidad (Delta Q)
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
                double D = computeModularityGain(k_i, sigma_tot_i, sigma_tot_j, k_i_in_i, k_i_in_j);

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

} // namespace networkStructure
