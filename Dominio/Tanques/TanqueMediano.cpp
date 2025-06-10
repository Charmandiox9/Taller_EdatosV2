#include "TanqueMediano.h"

TanqueMediano::TanqueMediano(int id) : Tanque(id){

    danio = 150;
    movimientoBase = 4;
    vida = 250;
}

int TanqueMediano::obtenerVidaPorTipo(){
    return 250;
}

int TanqueMediano::getDanio() const{
    return danio;
}

int TanqueMediano::getMovimientoBase() const{
    return movimientoBase;
}

Tanque* TanqueMediano::clonar() const {
    return new TanqueMediano(*this);
}
