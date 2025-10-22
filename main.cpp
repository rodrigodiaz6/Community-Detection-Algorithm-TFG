#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include "Network.h" 
#include "Node.h"
#include "Edge.h"
#include "Algoritmo.h"

using namespace networkStructure;

/**
 * @brief Carga una red desde un archivo CSV.
 * @param filename Nombre del archivo CSV.
 * @param network Referencia a un objeto Network donde se cargará la red.
 * @return true si la carga fue exitosa, false en caso contrario.
 */
bool loadNetworkFromCSV(const std::string& filename, Network& network) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << filename << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string origin_str, destiny_str, weight_str;

        if (std::getline(ss, origin_str, ',') &&
            std::getline(ss, destiny_str, ',') &&
            std::getline(ss, weight_str)) {
            try {
                unsigned int origin_id = std::stoul(origin_str);
                unsigned int destiny_id = std::stoul(destiny_str);
                double weight = std::stod(weight_str);
                network.addEdge(origin_id, destiny_id, weight);
            } catch (const std::exception& e) {
                std::cerr << "Advertencia: Se omitió una línea por formato inválido: " << line << std::endl;
            }
        }
    }
    file.close();
    return true;
}

/**
 * @brief Imprime todos los nodos y sus conexiones en la red.
 * @param network La red a imprimir.
 */
void printNetwork(Network& network) {
    std::cout << "\n--- Estado Actual de la Red ---" << std::endl;
    std::cout << "Nodos Totales: " << network.getNNodes() << " | Aristas Totales: " << network.getNEdges() << std::endl;
    std::cout << "---------------------------------" << std::endl;

    for (const auto& pair : network.getNodesMap()) {
        Node* node = pair.second.get();
        if (node) {
            std::cout << "Nodo " << node->getID() << " (Comunidad: " << node->getCommunity() << ", Grado: " << node->getDegree() << ") conectado a:" << std::endl;
            const auto& adjList = node->getAdjList();
            if (adjList.empty()) {
                std::cout << "  (Sin conexiones)" << std::endl;
            } else {
                for (const auto& edge : adjList) {
                    Node* opposite = edge->getOpposite(node);
                    std::cout << "  -> Nodo " << opposite->getID() 
                              << " (via Arista ID " << edge->getID() 
                              << ", Peso: " << edge->getWeight() << ")" << std::endl;
                }
            }
        }
    }
     std::cout << "---------------------------------" << std::endl;
}

void printCommunities(Network& network) {
    std::cout << "Estado de las comunidades:" << std::endl;
    std::map<int, std::vector<unsigned int>> communities;
    for (const auto& pair : network.getNodesMap()) {
        Node* node = pair.second.get();
        communities[node->getCommunity()].push_back(node->getID());
    }

    for(const auto& pair : communities){
        std::cout << "  - Comunidad " << pair.first << ": { ";
        for(unsigned int nodeId : pair.second){
            std::cout << nodeId << " ";
        }
        std::cout << "}" << std::endl;
    }
    std::cout << "---------------------------" << std::endl;
}

/**
 * @brief Muestra el menú de opciones al usuario.
 */
void showMenu() {
    std::cout << "\n--- Menu de Opciones ---" << std::endl;
    std::cout << "1. Eliminar un Nodo" << std::endl;
    std::cout << "2. Eliminar una Arista" << std::endl;
    std::cout << "3. Imprimir la Red Completa" << std::endl;
    std::cout << "4. Algoritmo de comunidades" << std::endl;
    std::cout << "5. Finalizar Ejecucion" << std::endl;
    std::cout << "Seleccione una opcion: ";
}

int main() {
    Network myNetwork;

    // Cargamos la red
    std::cout << "Cargando red desde 'network_data.csv'..." << std::endl;
    if (!loadNetworkFromCSV("network_data.csv", myNetwork)) {
        return 1; // Termina si no se puede cargar el archivo.
    }
    std::cout << "Red cargada con " << myNetwork.getNNodes() << " nodos y " 
              << myNetwork.getNEdges() << " aristas." << std::endl;

    int choice;
    while (true) {
        showMenu();
        std::cin >> choice;

        // Validamos la entrada del usuario
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada inválida. Por favor, introduce un número." << std::endl;
            continue;
        }

        if (choice == 1) {
            // Eliminar Nodo
            unsigned int node_id;
            std::cout << "Introduce el ID del nodo a eliminar: ";
            std::cin >> node_id;
            if (myNetwork.getNode(node_id)) {
                myNetwork.removeNode(node_id);
                std::cout << "Nodo " << node_id << " y sus aristas han sido eliminados." << std::endl;
            } else {
                std::cout << "Error: El nodo " << node_id << " no existe." << std::endl;
            }
        } else if (choice == 2) {
            // Eliminar Arista
            unsigned int edge_id;
            std::cout << "Introduce el ID de la arista a eliminar: ";
            std::cin >> edge_id;
            if (myNetwork.getEdge(edge_id)) {
                myNetwork.removeEdge(edge_id);
                std::cout << "Arista " << edge_id << " eliminada." << std::endl;
            } else {
                std::cout << "Error: La arista " << edge_id << " no existe." << std::endl;
            }
        } else if (choice == 3) {
            // Imprimir red completa
            printNetwork(myNetwork);
        } else if (choice == 4) {
            // Ejecutar algoritmo de comunidades
            std::cout << "Ejecutando algoritmo de deteccion de comunidades..." << std::endl;
            printCommunities(myNetwork);
            Algoritmo algoritmo(&myNetwork);
            algoritmo.run(0.000001); // Umbral de ganancia de modularidad
            std::cout << "Algoritmo completado. Comunidades asignadas." << std::endl;
            printCommunities(myNetwork);
        } else if (choice == 5) {
            // Salir
            std::cout << "Finalizando ejecucion." << std::endl;
            break;
        } else {
            std::cout << "Opcion no válida. Inténtalo de nuevo." << std::endl;
        }
    }
    return 0;
}