#include "TanquePesado.h"

TanquePesado::TanquePesado(int id) : Tanque(id) {
    daño = 200;
    movimientoBase = 2;
    vida = 350; // Ajuste para evitar el constructor base que usa obtenerVidaPorTipo()
}

int TanquePesado::obtenerVidaPorTipo() {
    return 350;
}

int TanquePesado::getDaño() const {
    return daño;
}

int TanquePesado::getMovimientoBase() const {
    return movimientoBase;
}