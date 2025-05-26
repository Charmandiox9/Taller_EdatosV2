#ifndef TANQUEMEDIANO_H
#define TANQUEMEDIANO_H

#include "Tanque.h"

class TanqueMediano : public Tanque{

private:
    int danio;
    int movimientoBase;

public:
    TanqueMediano(int id);

    int obtenerVidaPorTipo() override;

    int getDanio() const;
    int getMovimientoBase() const;

};

#endif