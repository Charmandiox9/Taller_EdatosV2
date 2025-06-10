#include "TanquePesado.h"

TanquePesado::TanquePesado(int id) : Tanque(id) {
    danio = 200;
    movimientoBase = 2;
    vida = 350;
}

int TanquePesado::obtenerVidaPorTipo() {
    return 350;
}

int TanquePesado::getDanio() const {
    return danio;
}

int TanquePesado::getMovimientoBase() const {
    return movimientoBase;
}

Tanque* TanquePesado::clonar() const {
    return new TanquePesado(*this);
}

