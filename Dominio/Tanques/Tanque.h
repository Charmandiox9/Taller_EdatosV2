#ifndef TANQUE_H
#define TANQUE_H

class Tanque {
protected:
    int idTanque;
    int vida;
    int daño;
    int movimientoBase;

public:
    Tanque(int id);
    virtual int obtenerVidaPorTipo();
    void actualizarVida(int cantidad);
    double getProbabilidadDeImpacto(int tipoTerrenoInicio, int tipoTerrenoFinal) const;

    int getIdTanque() const;
    int getVida() const;
    virtual int getDaño() const;
    virtual int getMovimientoBase() const;

    void setId(int nuevoId);
    void setVida(int nuevaVida);
    virtual void setDaño(int nuevoDaño);
    virtual void setMovimientoBase(int nuevoMovimientoBase);

    virtual ~Tanque() = default;
};

#endif