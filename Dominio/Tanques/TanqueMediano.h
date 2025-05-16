#ifndef TANQUEMEDIANO_H
#define TANQUEMEDIANO_H

#include "Tanque.h"

class TanqueMediano : public Tanque{

private:
    int daño;
    int movimientoBase;

public:
    TanqueMediano(int id);

    int obtenerVidaPorTipo() override;

    int getDaño() const;
    int getMovimientoBase() const;

};

#endif