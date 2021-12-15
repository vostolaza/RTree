#include "RTree.h"
#include <chrono>
using namespace std::chrono;
using namespace std;

int main() {
    RTree rtree(100, 500);
    cout << "Insertando datos del mes de Enero y Febrero" << endl;
    auto start = high_resolution_clock::now();
    rtree.loadFromCSV("green_tripdata_2015-01.csv");
    rtree.loadFromCSV("green_tripdata_2015-02.csv");
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    cout << "Duracion de insercion: " << duration.count() << " segundos" << endl;
    cout << "Puntos insertados: " << rtree.size() << endl;
    cout << "Inserciones por segundo: " << rtree.size() / duration.count() << endl;
    Point ll{-73.5, 40};
    Point ur{-73, 41};
    cout << "Comenzando busqueda por rango: " << ll << " " << ur << endl;
    start = high_resolution_clock::now();
    auto res = rtree.query(ll, ur);
    stop = high_resolution_clock::now();
    auto duration2 = duration_cast<microseconds>(stop - start);
    cout << "Duracion de consulta: " << duration2.count() << " microsegundos" << endl;
    cout << "Numero de puntos en el rango: " << res.size() << endl;
    return 0;
}
