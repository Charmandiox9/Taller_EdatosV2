#ifndef NODOSISTEMA_H
#define NODOSISTEMA_H

#include "../Tanques/Tanque.h"

class NodoSistema {
private:
    int idNodo;
    int posicionX;
    int posicionY;
    int tipoTerreno; // 1: planicie, 2: bosque, 3: montaña, 0: agua

    Tanque* tanque; // Puntero al tanque ubicado en este nodo (puede ser nullptr)
    NodoSistema* siguiente;
    NodoSistema* anterior;

public:
    NodoSistema(int id, int x, int y, int tipoTerreno);

    // Getters
    int getIdNodo() const;
    int getPosX() const;
    int getPosY() const;
    int getTipoTerreno() const;
    Tanque* getTanque() const;
    NodoSistema* getSiguiente() const;
    NodoSistema* getAnterior() const;

    // Setters
    void setTanque(Tanque* nuevoTanque);
    void setPosX(int nuevoX);
    void setPosY(int nuevoY);
    void setTipoTerreno(int nuevoTipoTerreno);
    void setSiguiente(NodoSistema* siguienteNodo);
    void setAnterior(NodoSistema* anteriorNodo);

    // Gestión de memoria
    void destruirTanque(); // Elimina el tanque asociado solo si fue creado dentro del nodo
};

#endif