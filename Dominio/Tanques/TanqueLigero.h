#ifndef TANQUELIGERO_H
#define TANQUELIGERO_H

#include "Tanque.h"

class TanqueLigero : public Tanque{

private:
    int danio;
    int movimientoBase;

public:
    TanqueLigero(int id);

    int obtenerVidaPorTipo() override;

    int getDanio() const;
    int getMovimientoBase() const;

};

#endif