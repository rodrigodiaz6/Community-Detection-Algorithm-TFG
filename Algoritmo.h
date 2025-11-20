#ifndef ALGORITMO_H
#define ALGORITMO_H

#include "Network.h"
#include "Node.h"
#include "Edge.h"

#include <map>
#include <vector>
#include <memory>

namespace networkStructure {
/**
 * @class Algoritmo
 * @brief Implementa la detección de comunidades mediante el Constant Potts Model (CPM).
 * @details Esta clase aplica una optimización local basada en el criterio CPM.
 * En cada iteración analiza los nodos de la red y los desplaza a la comunidad vecina
 * que proporcione la mayor mejora en la función de calidad del modelo. El proceso
 * continúa hasta que no se producen más movimientos que incrementen dicha calidad.
 */
class Algoritmo {
public:
    /**
     * @brief Constructor de la clase.
     * @param net Puntero a la red (Network) sobre la que se ejecutará el algoritmo.
     */
    Algoritmo(networkStructure::Network* net);

    /**
     * @brief Ejecuta el algoritmo de detección de comunidades usando Constant Potts Model (CPM).
     * @param min_gain Umbral mínimo de ganancia de calidad para aceptar un movimiento.
     * @param gamma Parámetro de resolución del CPM (controla el tamaño de las comunidades).
     */
    void run(double min_gain = 0, double gamma = 1.0);


        /**
     * @brief Fusiona los nodos que pertenecen a la misma comunidad en nodos únicos.
     * @details Implementa el pseudocódigo mergeCommunities(G):
     *  - Agrupa los nodos por su atributo community.
     *  - Para cada comunidad con tamaño > 1, crea un nodo nuevo que representa a dicha comunidad.
     *  - Acumula los pesos de las aristas hacia nodos fuera de la comunidad (externalWeights).
     *  - Crea aristas desde el nodo fusionado hacia cada vecino externo con el peso total acumulado.
     *  - Elimina los nodos originales de esa comunidad.
     * 
     * Complejidad: O(m), siendo m el número de aristas de la red.
     */
    void mergeCommunities();

private:
    networkStructure::Network* network; ///< Puntero a la red que se está procesando.
    /**
     * @brief Asigna a cada nodo su propia comunidad única.
     * @details Sobrescribe la inicialización por defecto de la clase Node (que asigna 1).
     * Cada nodo 'i' se asigna a la comunidad 'i'.
     */
    void initializeCommunities();

    /**
     * @brief Obtiene los pesos de las aristas de un nodo hacia cada comunidad vecina.
     * @param node El nodo a inspeccionar.
     * @return Un mapa donde la clave es el ID de la comunidad vecina y el valor es la suma de pesos de las aristas a esa comunidad (k_i_in).
     */
    std::map<int, double> getNeighborCommunityWeights(networkStructure::Node* node);
};

} // namespace networkStructure

#endif // ALGORITMO_H
