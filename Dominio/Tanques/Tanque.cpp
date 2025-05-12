#include "Tanque.h"

Tanque::Tanque(int id) : idTanque(id), vida(obtenerVidaPorTipo()) {}

int Tanque::obtenerVidaPorTipo() {
    return 100;
}

void Tanque::actualizarVida(int cantidad) {
    vida -= cantidad;
    if (vida < 0) vida = 0;
}

double Tanque::getProbabilidadDeImpacto(int tipoTerrenoInicio, int tipoTerrenoFinal) const {
    double probabilidad = 0.0;

    if (tipoTerrenoInicio == 0 || tipoTerrenoFinal == 0) {
        probabilidad = 0.0; // Agua
    } else if (tipoTerrenoInicio == 1 ) {
        if(tipoTerrenoFinal == 1){
            probabilidad = 1.0; // Planicie a planicie
        } else if(tipoTerrenoFinal == 2){
            probabilidad = 1.0 * 0.75; // Planicie a bosque
        } else if(tipoTerrenoFinal == 3){
            probabilidad = 1.0 * 0.4; // Planicie a montaña
        }
    } else if (tipoTerrenoInicio == 2) {
        if(tipoTerrenoFinal == 1){
            probabilidad = 0.75 * 1.0; // Bosque a planicie
        } else if(tipoTerrenoFinal == 2){
            probabilidad = 0.75 * 0.75; // Bosque a bosque
        } else if(tipoTerrenoFinal == 3){
            probabilidad = 0.75 * 0.4; // Bosque a montaña
        }
    } else if (tipoTerrenoInicio == 3) {
        if(tipoTerrenoFinal == 1){
            probabilidad = 0.4 * 1.0; // Montaña a planicie
        } else if(tipoTerrenoFinal == 2){
            probabilidad = 0.4 * 0.75; // Montaña a bosque
        } else if(tipoTerrenoFinal == 3){
            probabilidad = 0.4 * 0.4; // Montaña a montaña
        }
    } 
    return probabilidad;
}

int Tanque::getIdTanque() const {
    return idTanque;
}

int Tanque::getVida() const {
    return vida;
}

void Tanque::setId(int nuevoId) {
    idTanque = nuevoId;
}

void Tanque::setVida(int nuevaVida) {
    vida = nuevaVida;
}