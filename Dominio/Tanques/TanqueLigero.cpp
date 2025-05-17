#include "TanqueLigero.h"

TanqueLigero::TanqueLigero(int id) : Tanque(id){

    daño = 100;
    movimientoBase = 6;
    vida = 200; 
}

int TanqueLigero::obtenerVidaPorTipo(){
    return 200;
}

int TanqueLigero::getDaño() const{
    return daño;
}

int TanqueLigero::getMovimientoBase() const{
    return movimientoBase;
}