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
#include <omp.h> 

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
    for (const auto& pair : network.getNodesMap()) {
        Node* node = pair.second.get();
        if (!node) continue;
        std::cout << "Nodo " << node->getID() << " (Comunidad: " << node->getCommunity() << ", Grado: " << node->getDegree() << ")" << std::endl;
        const auto& members = node->getMembers();
        if (!members.empty()) {
            std::cout << "  Miembros: ";
            for (unsigned int mid : members) {
                std::cout << mid << " ";
            }
            std::cout << std::endl;
        } else {
            std::cout << "  Miembros: (ninguno)" << std::endl;
        }
        std::cout << "  Conectado a:" << std::endl;
        const auto& adjList = node->getAdjList();
        if (adjList.empty()) {
            std::cout << "    (Sin conexiones)" << std::endl;
        } else {
            for (const auto& edge : adjList) {
                Node* opposite = edge->getOpposite(node);
                std::cout << "    -> Nodo " << opposite->getID() << " (via Arista ID " << edge->getID() << ", Peso: " << edge->getWeight() << ")" << std::endl;
            }
        }
    }
}

void printNetworkLite(Network& network) {
    std::cout << "\n--- Resumen de la Red ---" << std::endl;
    std::cout << "Nodos Totales: " << network.getNNodes() << " | Aristas Totales: " << network.getNEdges() << std::endl;
    for (const auto& pair : network.getNodesMap()) {
        Node* node = pair.second.get();
        if (!node) continue;
        const auto& members = node->getMembers();
        std::size_t numMembers = members.size();
        std::cout << "Nodo " << node->getID() << ": " << numMembers << " miembros" << std::endl;
    }
}

void printCommunities(Network& network) {
    std::map<int, unsigned int> communitySizes;
    // Contar cuántos nodos hay en cada comunidad
    for (const auto& pair : network.getNodesMap()) {
        Node* node = pair.second.get();
        if (!node) continue;
        int commId = node->getCommunity();
        communitySizes[commId]++;   // sumamos 1 nodo a esa comunidad
    }
    std::cout << "Estado de las comunidades:" << std::endl;
    std::cout << "Numero de comunidades: " << communitySizes.size() << std::endl;
    for (const auto& entry : communitySizes) {
        int commId = entry.first;
        unsigned int size = entry.second;
        std::cout << "  - Comunidad " << commId << ": " << size << " nodos" << std::endl;
    }
}

/**
 * @brief Muestra el menú de opciones al usuario.
 */
void showMenu() {
    std::cout << "\n--- Menu de Opciones ---" << std::endl;
    std::cout << "1. Imprimir la Red Completa" << std::endl;
    std::cout << "2. Algoritmo de comunidades" << std::endl;
    std::cout << "3. Fusionar nodos por comunidades" << std::endl;
    std::cout << "4. Finalizar Ejecucion" << std::endl;
    std::cout << "Seleccione una opcion: ";
}

int main() {
    Network myNetwork;
     // Configurar número de hilos para OpenMP
    int num_threads;
    std::cout << "Introduce el numero de hilos a utilizar (1-16): ";
    std::cin >> num_threads;
    omp_set_num_threads(num_threads);
    // Cargamos la red
    std::cout << "Cargando red..." << std::endl;
    if (!loadNetworkFromCSV("Test4001_Rodrigo.csv", myNetwork)) {
        return 1; // Termina si no se puede cargar el archivo.
    }
    std::cout << "Red cargada con " << myNetwork.getNNodes() << " nodos y " << myNetwork.getNEdges() << " aristas." << std::endl;

    int choice;
    while (true) {
        showMenu();
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada inválida. Por favor, introduce un número." << std::endl;
            continue;
        }

        if (choice == 1) { // Mostrar red
            printNetwork(myNetwork);
        } else if (choice == 2) { // Ejecutar algoritmo de comunidades
            std::cout << "Ejecutando algoritmo de deteccion de comunidades..." << std::endl;
            Algoritmo algoritmo(&myNetwork);
            double t0 = omp_get_wtime();
            algoritmo.run(0.000001, 0.001); // min_gain, gamma
            double t1 = omp_get_wtime();
            std::cout << "Algoritmo completado. Comunidades asignadas." << std::endl;
            printCommunities(myNetwork);
            std::cout << "Tiempo de ejecucion del algoritmo: " << (t1 - t0) << " segundos." << std::endl;
        } else if (choice == 3) { // Fusionar nodos por comunidades
            Algoritmo algoritmo(&myNetwork);
            algoritmo.mergeCommunities();
            std::cout << "Nodos fusionados por comunidades." << std::endl;
            printNetworkLite(myNetwork);
        } else if (choice == 4) { //Salir
            std::cout << "Finalizando ejecucion." << std::endl;
            break;
        } else {
            std::cout << "Opcion no válida. Inténtalo de nuevo." << std::endl;
        }
    }
    return 0;
}