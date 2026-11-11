#include <exception>
#include <iomanip>
#include <iostream>

#include "pricing/pricing.h"

int main() {
    // Ejemplo de referencia de Hull. Campos: spot, strike, rate, vol, time
    const pricing::BSParams params{100.0, 100.0, 0.05, 0.2, 1.0};

    // Las funciones de la librería lanzan excepciones si los parámetros no son
    // válidos, así que el programa las captura y muestra el error en lugar de
    // terminar abruptamente.
    try {
        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Call price: " << pricing::bs_call(params) << '\n';
        std::cout << "Put price:  " << pricing::bs_put(params) << '\n';
        std::cout << "Call delta: " << pricing::delta_call(params) << '\n';
        std::cout << "Put delta:  " << pricing::delta_put(params) << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
