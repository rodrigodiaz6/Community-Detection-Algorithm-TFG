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
 * @brief Implementa el algoritmo de detección de comunidades.
 * @details Esta clase realiza la optimización local de la modularidad.
 * Itera sobre los nodos de una red y los mueve a la comunidad vecina que proporcione la mayor ganancia de modularidad positiva.
 * El proceso se repite hasta que no se puedan realizar más movimientos que aumenten la modularidad.
 */
class Algoritmo {

public:
    /**
     * @brief Constructor de la clase.
     * @param net Puntero a la red (Network) sobre la que se ejecutará el algoritmo.
     */
    Algoritmo(networkStructure::Network* net);

    /**
     * @brief Ejecuta el algoritmo de detección de comunidades.
     * @details
     * 1. Inicializa cada nodo en su propia comunidad.
     * 2. Repite el proceso de asignación de comunidades hasta que no haya cambios:
     * a. Precalcula los agregados (grados de nodos, grados de comunidades, peso total).
     * b. Itera sobre todos los nodos en orden aleatorio.
     * c. Para cada nodo, calcula la ganancia de modularidad de moverlo a la comunidad de cada uno de sus vecinos.
     * d. Almacena el movimiento que da la mayor ganancia positiva.
     * e. Después de comprobar todos los nodos, aplica todos los movimientos almacenados.
     * 3. El bucle se detiene cuando una pasada completa no produce ningún cambio de comunidad.
     *
     * @param min_modularity_gain El umbral de ganancia mínima para considerar un movimiento válido.
     * Un movimiento solo se considera si su ganancia de modularidad excede este valor. El valor común es 0.000001.
     */
    void run(double min_modularity_gain = 0.000001);

private:
    networkStructure::Network* network; ///< Puntero a la red que se está procesando.
    double m; ///< El peso total de todas las aristas en la red (sum(W_ij) / 2).

    // Mapas para almacenar los valores agregados necesarios para el cálculo de modularidad.
    std::map<unsigned int, double> node_degrees; ///< k_i: Grado ponderado de cada nodo (ID de nodo -> k_i).
    std::map<int, double> community_degrees;     ///< Sigma_tot: Grado ponderado total de cada comunidad (ID de comunidad -> Sigma_tot).

    /**
     * @brief Asigna a cada nodo su propia comunidad única.
     * @details Sobrescribe la inicialización por defecto de la clase Node (que asigna 1).
     * Cada nodo 'i' se asigna a la comunidad 'i'.
     */
    void initializeCommunities();

    /**
     * @brief Calcula y almacena el peso total (m), los grados de los nodos (k_i) y los grados de las comunidades (Sigma_tot).
     * @details Se debe llamar al inicio de cada pasada del bucle principal, ya que los Sigma_tot cambian cuando los nodos se mueven.
     */
    void precomputeAggregates();

    /**
     * @brief Calcula la ganancia de modularidad (Delta Q) al mover un nodo de su comunidad actual a una comunidad objetivo.
     * @details La fórmula usada es la estándar para el cambio de modularidad (Delta Q) al mover un nodo 'i' de una comunidad 'C' a una 'D':
     * Delta Q = [ (k_i_in_D - k_i_in_C) / m ] + [ (k_i * (Sigma_tot_C - Sigma_tot_D - k_i)) / (2*m*m) ]
     *
     * @param k_i Grado ponderado del nodo que se mueve.
     * @param sigma_tot_C Grado ponderado total de la comunidad *actual* (C).
     * @param sigma_tot_D Grado ponderado total de la comunidad *objetivo* (D).
     * @param k_i_in_C Peso de las aristas de 'i' a otros nodos en 'C'.
     * @param k_i_in_D Peso de las aristas de 'i' a otros nodos en 'D'.
     * @return El valor de Delta Q (D).
     */
    double computeModularityGain(double k_i, double sigma_tot_C, double sigma_tot_D, double k_i_in_C, double k_i_in_D);

    /**
     * @brief Obtiene los pesos de las aristas de un nodo hacia cada comunidad vecina.
     * @param node El nodo a inspeccionar.
     * @return Un mapa donde la clave es el ID de la comunidad vecina y el valor es la suma de pesos de las aristas a esa comunidad (k_i_in).
     */
    std::map<int, double> getNeighborCommunityWeights(networkStructure::Node* node);
};

} // namespace networkStructure

#endif // ALGORITMO_H
