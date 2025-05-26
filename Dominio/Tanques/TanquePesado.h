#ifndef TANQUEPESADO_H
#define TANQUEPESADO_H

#include "Tanque.h"

class TanquePesado : public Tanque {
private:
    int danio;
    int movimientoBase;

public:
    TanquePesado(int id);

    int obtenerVidaPorTipo() override;

    int getDanio() const;
    int getMovimientoBase() const;
};

#endif