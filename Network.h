#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <map>
#include <memory>
#include "Node.h"
#include "Edge.h"

namespace networkStructure {

class Network {
private:
    // Usamos unique_ptr para la gestión automática de memoria de nodos y aristas.
    std::map<unsigned int, std::unique_ptr<Node>> nodes;
    std::map<unsigned int, std::unique_ptr<Edge>> edges;
    unsigned int next_edge_id = 0; // Contador para los IDs de arista únicos.

public:
    /**
     * @brief Constructor y destructor de la clase Network.
     */
    Network() = default;
    ~Network() = default;

    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;
    Network(Network&&) = default;
    Network& operator=(Network&&) = default;

    /**
     * @brief Devuelve el número total de nodos en la red.
     * @return El tamaño del mapa de nodos.
     */
    std::size_t getNNodes();

    /**
     * @brief Devuelve el número total de aristas en la red.
     * @return El tamaño del mapa de aristas.
     */
    std::size_t getNEdges();

    /**
     * @brief Busca y devuelve un puntero a un nodo por su ID.
     * @param id El ID del nodo a buscar.
     * @return Puntero al nodo si se encuentra, de lo contrario nullptr.
     */
    Node* getNode(unsigned int id);

    /**
     * @brief Busca y devuelve un puntero a una arista por su ID.
     * @param id El ID de la arista a buscar.
     * @return Puntero a la arista si se encuentra, de lo contrario nullptr.
     */
    Edge* getEdge(unsigned int id);

    /**
     * @brief Añade un nuevo nodo a la red si no existe.
     * @param id El ID del nodo a añadir.
     * @return Puntero al nodo nuevo o al ya existente.
     */
    Node* addNode(unsigned int id);

    /**
     * @brief Añade una arista entre dos nodos.
     * @details Si los nodos no existen, se crean. Permite aristas paralelas.
     * @param id_origin ID del nodo de origen.
     * @param id_destiny ID del nodo de destino.
     * @param weight Peso de la arista.
     * @return Puntero a la arista creada.
     */
    Edge* addEdge(unsigned int id_origin, unsigned int id_destiny, double weight);

    /**
     * @brief Elimina una arista de la red por su ID.
     * @details También la elimina de la lista de adyacencia de los nodos conectados.
     * @param id El ID de la arista a eliminar.
     */
    void removeEdge(unsigned int id);

    /**
     * @brief Elimina un nodo de la red por su ID.
     * @details Elimina también todas las aristas incidentes al nodo.
     * @param id El ID del nodo a eliminar.
     */
    void removeNode(unsigned int id);

    /**
     * @brief Obtiene una lista de todas las aristas incidentes a un nodo.
     * @param id El ID del nodo.
     * @return Un vector de punteros a las aristas. Si el nodo no existe, devuelve un vector vacío.
     */
    std::vector<Edge*> getEdgesOfNode(unsigned int id);

    /**
    * @brief Devuelve una referencia constante al mapa de nodos.
    * @details Se utiliza para permitir la iteración sobre todos los nodos de la red.
    * @return Una referencia constante al mapa que almacena los nodos.
    */
    const std::map<unsigned int, std::unique_ptr<Node>>& getNodesMap() const {
    return nodes;
    }

    /**
    * @brief Devuelve una referencia constante al mapa de aristas.
    * @details Permite la iteración segura sobre todas las aristas de la red.
    * @return Una referencia constante al mapa que almacena las aristas.
    */
    const std::map<unsigned int, std::unique_ptr<Edge>>& getEdgesMap() const {
        return edges;
    }
};

} // namespace networkStructure

#endif // NETWORK_H