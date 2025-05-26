#include "TanqueLigero.h"

TanqueLigero::TanqueLigero(int id) : Tanque(id){

    danio = 100;
    movimientoBase = 6;
    vida = 200; 
}

int TanqueLigero::obtenerVidaPorTipo(){
    return 200;
}

int TanqueLigero::getDanio() const{
    return danio;
}

int TanqueLigero::getMovimientoBase() const{
    return movimientoBase;
}