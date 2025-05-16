#ifndef TANQUELIGERO_H
#define TANQUELIGERO_H

#include "Tanque.h"

class TanqueLigero : public Tanque{

private:
    int daño;
    int movimientoBase;

public:
    TanqueLigero(int id);

    int obtenerVidaPorTipo() override;

    int getDaño() const;
    int getMovimientoBase() const;

};

#endif