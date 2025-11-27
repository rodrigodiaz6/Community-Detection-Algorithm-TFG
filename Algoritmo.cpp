#include "Algoritmo.h"
#include <vector>
#include <map>
#include <algorithm> 
#include <random>    
#include <numeric>   
#include <iostream>  
#include <unordered_map>
#include <unordered_set>
#include <omp.h>
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

    // Vector de nodos a procesar desde 0 hasta N-1
    std::vector<Node*> nodes_to_process;
    nodes_to_process.reserve(network->getNNodes());
    for (const auto& pair : network->getNodesMap()) {
        nodes_to_process.push_back(pair.second.get());
    }
    if (nodes_to_process.empty()) return;

    // Cálculo de grados (k_i) y 2m = suma total de grados
    std::vector<double> node_degrees(nodes_to_process.size(), 0.0);
    double total_degree = 0.0;

    for (std::size_t i = 0; i < nodes_to_process.size(); ++i) {
        Node* node = nodes_to_process[i];
        if (!node) continue;

        std::vector<Edge*> edges_of_node = network->getEdgesOfNode(node->getID());
        double k_i = 0.0;
        for (Edge* e : edges_of_node) {
            if (!e) continue;
            k_i += e->getWeight();
        }
        node_degrees[i] = k_i;
        total_degree += k_i;
    }

    if (total_degree == 0.0) {
        return;
    }

    // Algoritmo de planificación de carga para dividir nodos entre hilos
    int P = omp_get_max_threads();
    if (P < 1) P = 1;

    std::vector<int> inicial(P, 0);
    std::vector<int> final_idx(P, 0);

    double load = total_degree / static_cast<double>(P);
    int l = 0;
    double suma = 0.0;
    inicial[0] = 0;

    int N = static_cast<int>(nodes_to_process.size());

    for (int i = 0; i < N && l < P; ++i) {
        if (suma < load) {
            suma += node_degrees[i];
        } else {
            final_idx[l] = i;
            ++l;
            if (l < P) {
                inicial[l] = i;
            }
            suma = 0.0;
        }
    }

    if (l < P) {
        final_idx[l] = N;
        for (int t = l + 1; t < P; ++t) {
            inicial[t] = final_idx[t] = N;
        }
    } else {
        final_idx[P - 1] = N;
    }

    // Estructura para guardar el mejor cambio
    struct Change {
        int jaux;   // ID del nodo a mover
        int kaux;   // comunidad destino
        double dQ;  // ganancia de calidad
    };
    std::vector<Change> changeData(P);

    bool improved;

    // Bucle principal tipo SPLICE: en cada iteración solo se aplica el mejor movimiento global.
    do {
        improved = false;

        // Tamaños de comunidad (community_sizes[comm] = nº nodos)
        std::map<int, unsigned int> community_sizes;
        for (const auto& pair : network->getNodesMap()) {
            Node* node = pair.second.get();
            if (!node) continue;
            int comm = node->getCommunity();
            community_sizes[comm] += 1;
        }
        if (community_sizes.empty()) break;

        // Inicializamos los datos de cambio de cada hilo
        for (int i = 0; i < P; ++i) {
            changeData[i].jaux = -1;
            changeData[i].kaux = -1;
            changeData[i].dQ   = 0.0;
        }

        // Paralelo: cada hilo busca su mejor movimiento local
        #pragma omp parallel
        {
            int tid = omp_get_thread_num();
            int from = (tid < P ? inicial[tid]   : 0);
            int to   = (tid < P ? final_idx[tid] : 0);

            int   best_node_id   = -1;
            int   best_comm_dest = -1;
            double best_dQ       = 0.0;

            for (int idx = from; idx < to; ++idx) {
                Node* currentNode = nodes_to_process[idx];
                if (!currentNode) continue;

                int current_comm = currentNode->getCommunity();

                // tamaño de la comunidad actual
                unsigned int size_i = 0;
                auto it_size_i = community_sizes.find(current_comm);
                if (it_size_i != community_sizes.end()) {
                    size_i = it_size_i->second;
                }

                // pesos hacia cada comunidad vecina
                std::map<int, double> neighbor_comm_weights = getNeighborCommunityWeights(currentNode);

                // k_i_in_i: peso de aristas de i dentro de su propia comunidad actual
                double k_i_in_i = 0.0;
                auto it_self = neighbor_comm_weights.find(current_comm);
                if (it_self != neighbor_comm_weights.end()) {
                    k_i_in_i = it_self->second;
                }

                // Recorremos comunidades vecinas para ver a cuál moverlo
                for (const auto& entry : neighbor_comm_weights) {
                    int comm_j = entry.first;
                    double k_i_in_j = entry.second;

                    if (comm_j == current_comm) continue;

                    unsigned int size_j = 0;
                    auto it_size_j = community_sizes.find(comm_j);
                    if (it_size_j != community_sizes.end()) {
                        size_j = it_size_j->second;
                    }
                    // ΔQ según CPM para mover 'currentNode' de 'current_comm' a 'comm_j'
                    double dQ = (k_i_in_j - k_i_in_i) + gamma * (static_cast<double>(size_i) - static_cast<double>(size_j) - 1.0);
                    // Criterio SPLICE: nos quedamos con el mejor ΔQ del hilo
                    if (dQ - best_dQ > min_gain) {
                        best_dQ       = dQ;
                        best_node_id  = static_cast<int>(currentNode->getID());
                        best_comm_dest = comm_j;
                    }
                }
            }
            // Guardamos el mejor cambio encontrado por este hilo
            if (tid < P) {
                changeData[tid].jaux = best_node_id;
                changeData[tid].kaux = best_comm_dest;
                changeData[tid].dQ   = best_dQ;
            }
        } // fin región paralela

        // Elegimos el mejor movimiento global entre todos los hilos
        int pmax = -1;
        double dQmax = 0.0;
        for (int i = 0; i < P; ++i) {
            if (changeData[i].dQ > dQmax) {
                dQmax = changeData[i].dQ;
                pmax = i;
            }
        }
        // Aplicamos localMove si hay mejora positiva
        if (pmax != -1 && dQmax > 0.0 && changeData[pmax].jaux != -1 && changeData[pmax].kaux != -1) {
            Node* node_to_move = network->getNode(static_cast<unsigned int>(changeData[pmax].jaux));
            if (node_to_move) {
                node_to_move->setCommunity(changeData[pmax].kaux);
                improved = true;
            }
        } else {
            improved = false;
        }
    } while (improved);
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
