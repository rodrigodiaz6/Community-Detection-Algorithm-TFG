#ifndef NODE_H
#define NODE_H

#include <vector>
#include "Edge.h"

namespace networkStructure {
class Edge; 

/**
 * @class Node
 * @brief Representa un nodo en la red.
 * @details El nodo almacena su identificador, la comunidad a la que pertenece y las aristas que conectan con él. 
 */
class Node {
private:
    unsigned int id; ///< Identificador único del nodo.
    int community; ///< ID de la comunidad a la que pertenece el nodo.
    std::vector<Edge*> adjList; ///< Lista de punteros a las aristas incidentes.
    std::vector<unsigned int> members; ///< IDs de nodos si este nodo representa una comunidad fusionada.

public:
    /**
     * @brief Crea un nodo con un ID.
     * @details La comunidad por defecto es 1.
     * @param id Identificador único para el nodo.
     */
    Node(unsigned int id);

    /**
     * @brief Devuelve el ID del nodo.
     * @return El identificador del nodo.
     */
    unsigned int getID();

    /**
     * @brief Devuelve la comunidad a la que está asignado el nodo.
     * @return El ID de la comunidad.
     */
    int getCommunity();

    /**
     * @brief Asigna el nodo a una comunidad.
     * @param c El nuevo ID de comunidad.
     */
    void setCommunity(int c);

    /**
     * @brief Devuelve el grado del nodo.
     * @return El número de aristas incidentes al nodo.
     */
    std::size_t getDegree(); 

    /**
     * @brief Proporciona acceso de solo lectura a la lista de aristas.
     * @return Referencia constante al vector de punteros de aristas.
     */
    const std::vector<Edge*>& getAdjList();

    /**
     * @brief Comprueba si el nodo actual y el nodo dado son iguales según su id.
     * @param node Puntero a otro objeto Node.
     * @return true si los nodos son iguales, false en caso contrario.
     */
    bool equals(Node *node);

    /**
     * @brief Añade una arista a la lista de aristas del nodo actual.
     * @param edge Puntero a un objeto Edge.
     */
    virtual void addEdge(Edge *edge);

    /**
     * @brief Borra una arista de la lista de aristas del nodo actual.
     * @param edge Puntero a un objeto Edge.
     */
    virtual void eraseEdge(Edge *edge);

    /**
     * @brief Borra todas las aristas de la lista de aristas del nodo actual.
     */
    void eraseAllEdges();

    /**
     * @brief Añade un ID de nodo a la lista de nodos cuando se fusiona una comunidad.
     * @param member_id ID del nodo a añadir.
     */
    void addMember(unsigned int member_id);

    /**
     * @brief Devuelve los IDs de nodos originales contenidos en este nodo.
     */
    const std::vector<unsigned int>& getMembers() const;

    /**
     * @brief Permite establecer la lista de miembros del nodo.
     * @param new_members Vector con los IDs de los nodos.
     */
    void setMembers(const std::vector<unsigned int>& new_members);

    /**
     * @brief Destructor de la clase Node.
     */
    virtual ~Node();
};

} // namespace networkStructure

#endif // NODE_H