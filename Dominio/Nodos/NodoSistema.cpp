#include "NodoSistema.h"
#include "../Tanques/Tanque.h"
#include <iostream>

NodoSistema::NodoSistema(int id, int x, int y, int tipoTerreno)
    : idNodo(id),
      posicionX(x),
      posicionY(y),
      tipoTerreno(tipoTerreno),
      tanque(nullptr),
      siguiente(nullptr),
      anterior(nullptr) {}

int NodoSistema::getIdNodo() const {
    return idNodo;
}

int NodoSistema::getPosX() const {
    return posicionX;
}

int NodoSistema::getPosY() const {
    return posicionY;
}

int NodoSistema::getTipoTerreno() const {
    return tipoTerreno;
}

Tanque* NodoSistema::getTanque() const {
    return tanque;
}

NodoSistema* NodoSistema::getSiguiente() const {
    return siguiente;
}

NodoSistema* NodoSistema::getAnterior() const {
    return anterior;
}

void NodoSistema::setTanque(Tanque* nuevoTanque) {
    if (tanque != nullptr) {
        delete tanque; // Liberar memoria del tanque anterior
    }
    tanque = nuevoTanque;
}

void NodoSistema::setPosX(int nuevoX) {
    posicionX = nuevoX;
}

void NodoSistema::setPosY(int nuevoY) {
    posicionY = nuevoY;
}

void NodoSistema::setTipoTerreno(int nuevoTipoTerreno) {
    tipoTerreno = nuevoTipoTerreno;
}

void NodoSistema::setSiguiente(NodoSistema* siguienteNodo) {
    siguiente = siguienteNodo;
}

void NodoSistema::setAnterior(NodoSistema* anteriorNodo) {
    anterior = anteriorNodo;
}

// Destructor para liberar la memoria del tanque
void NodoSistema::destruirTanque() {
    if (tanque != nullptr) {
        delete tanque;
        tanque = nullptr;
    }
}
