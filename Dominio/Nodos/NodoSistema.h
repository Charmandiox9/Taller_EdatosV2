#ifndef NODOSISTEMA_H
#define NODOSISTEMA_H

#include "../Tanques/Tanque.h"

class NodoSistema{
    private:
        int idNodo;
        int posicionX;
        int posicionY;
        Tanque* tanque;
        int tipoTerreno; // 1: planicie, 2: bosque, 3: montaña, 0: agua
        NodoSistema* siguiente; // Puntero a siguiente nodo (opcional, si se necesita una lista enlazada)
        NodoSistema* anterior; // Puntero a nodo anterior (opcional, si se necesita una lista enlazada)
    public:
        NodoSistema(int id, int x, int y, int tipoTerreno);

        int getIdNodo() const;
        int getPosX() const;
        int getPosY() const;
        int getTipoTerreno() const;
        Tanque* getTanque() const;
        NodoSistema* getSiguiente() const; // Método para obtener el siguiente nodo
        NodoSistema* getAnterior() const; // Método para obtener el nodo anterior
        void destruirTanque();

        void setTanque(Tanque* nuevoTanque);
        void setPosX(int nuevoX);
        void setPosY(int nuevoY);
        void setTipoTerreno(int nuevoTipoTerreno);
        void setSiguiente(NodoSistema* siguienteNodo); // Método para establecer el siguiente nodo
        void setAnterior(NodoSistema* anteriorNodo); // Método para establecer el nodo anterior
};


#endif