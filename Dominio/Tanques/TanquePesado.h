#ifndef TANQUEPESADO_H
#define TANQUEPESADO_H

#include "Tanque.h"

class TanquePesado : public Tanque {
private:
    int daño;
    int movimientoBase;

public:
    TanquePesado(int id);

    int obtenerVidaPorTipo() override;

    int getDaño() const;
    int getMovimientoBase() const;
};

#endif