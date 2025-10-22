#ifndef EDGE_H
#define EDGE_H
#include "Node.h"

namespace networkStructure {
class Node;

/**
 * @class Edge
 * @brief Representa una arista entre dos nodos en la red.
 * @details Las aristas son no dirigidas por defecto, conectando un nodo 'n1' con otro 'n2'.
 * Almacenan un peso, que cuantifica la intensidad de la conexión.
 */
class Edge {
public:
    using id_t = unsigned int;

protected:
    unsigned int id;///< Identificador único de la arista
    Node *n1;///< Puntero al nodo.
    Node *n2;///< Puntero al nodo opuesto.
    double weight;///< Peso de la arista.

public:
    /**
     * @brief Método onstructor nueva arista.
     * @param id Identificador único para la arista.
     * @param n1 Puntero al nodo de origen.
     * @param n2 Puntero al nodo de destino.
     * @param w Peso de la arista (por defecto 1.0).
     */
    Edge(unsigned int id, Node* n1, Node* n2, double w = 1.0);

    /**
     * @brief Devuelve el identificador de la arista.
     * @return El id de la arista.
     */
    unsigned int getID();

    /**
     * @brief Devuelve el puntero al nodo de origen.
     * @return Puntero al nodo origen.
     */
    Node* getOrigin();

    /**
     * @brief Devuelve el puntero al nodo de destino.
     * @return Puntero al nodo destino.
     */
    Node* getDestiny();

    /**
     * @brief Devuelve el peso de la arista.
     * @return El valor del peso.
     */
    double getWeight();

    /**
     * @brief Dado un nodo que pertenece a la arista, devuelve el nodo opuesto.
     * @details Se usa para recorrer los vecinos de un nodo. Si el nodo 'n' es el origen, devuelve el destino y viceversa.
     * @param n Puntero a uno de los nodos de la arista.
     * @return Puntero al nodo opuesto, o nullptr si 'n' no está en el otro extremo de la arista.
     */
    Node *getOpposite(Node *n);

    /**
     * @brief Compara si dos aristas son iguales basándose en su ID.
     * @param other Puntero a otra arista.
     * @return true si los IDs son iguales, false en caso contrario.
     */
    bool equals(Edge* edge);
};
} // namespace networkStructure

#endif // EDGE_H