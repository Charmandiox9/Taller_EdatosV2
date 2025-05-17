#include "TanqueMediano.h"

TanqueMediano::TanqueMediano(int id) : Tanque(id){

    daño = 150;
    movimientoBase = 4;
    vida = 250;
}

int TanqueMediano::obtenerVidaPorTipo(){
    return 250;
}

int TanqueMediano::getDaño() const{
    return daño;
}

int TanqueMediano::getMovimientoBase() const{
    return movimientoBase;
}