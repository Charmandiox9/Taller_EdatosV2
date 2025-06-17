#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <stack>
#include <vector>
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()
#include <limits>
#include <cmath>

#include "../Dominio/Tanques/Tanque.h"
#include "../Dominio/Tanques/TanqueLigero.h"
#include "../Dominio/Tanques/TanqueMediano.h"
#include "../Dominio/Tanques/TanquePesado.h"
#include "../Dominio/Nodos/NodoSistema.h"
using namespace std;

enum TipoExplosion { NINGUNA, TERRENO, TANQUE };

struct Explosion {
    int x, y;
    float tiempoRestante;
    TipoExplosion tipo;
};

std::vector<Explosion> explosiones; //explosiones activas

TipoExplosion ultimaExplosion = NINGUNA;

const int INF = numeric_limits<int>::max();

enum Accion { MOVER, DISPARAR, ESPERAR };
// Representa el estado del juego
struct EstadoJuego {
    NodoSistema* tableroClonado; // Copia del tablero original
    Tanque* tanqueIA;
    Tanque* tanqueJugador;
    bool turnoIA;
    Accion accionAplicada;
    int posAccionX, posAccionY;

    EstadoJuego(NodoSistema* clon, Tanque* ia, Tanque* jugador, bool turno)
        : tableroClonado(clon), tanqueIA(ia), tanqueJugador(jugador), turnoIA(turno), 
        accionAplicada(ESPERAR), posAccionX(-1), posAccionY(-1) {}

    ~EstadoJuego() {
        // Liberar tablero clonado
        while (tableroClonado) {
            NodoSistema* temp = tableroClonado;
            tableroClonado = tableroClonado->getSiguiente();
            delete temp;
        }
        // Liberar tanques clonados
        delete tanqueIA;
        delete tanqueJugador;
    }
};


NodoSistema* clonarTablero(NodoSistema* original) {
    if (!original) return nullptr;

    NodoSistema* nuevoTablero = nullptr;
    NodoSistema* ultimo = nullptr;

    NodoSistema* actual = original;
    int id = 100;
    while (actual) {
        NodoSistema* nuevo = new NodoSistema(id++, actual->getPosX(), actual->getPosY(), actual->getTipoTerreno());
        /*nuevo->setPosX(actual->getPosX());
        nuevo->setPosY(actual->getPosY());
        nuevo->setTipoTerreno(actual->getTipoTerreno());*/

        // Clonamos el tanque si existe
        if (actual->getTanque()) {
            Tanque* originalTanque = actual->getTanque();
            Tanque* nuevoTanque = originalTanque->clonar(); // Supone constructor por copia
            nuevo->setTanque(nuevoTanque);
        } else {
            nuevo->setTanque(nullptr);
        }

        // Enlazar nodo a la lista nueva
        if (!nuevoTablero) {
            nuevoTablero = nuevo;
        } else {
            ultimo->setSiguiente(nuevo);
        }

        ultimo = nuevo;
        actual = actual->getSiguiente();
    }

    return nuevoTablero;
}

NodoSistema* buscarNodo(NodoSistema* tablero, int x, int y) {
    NodoSistema* actual = tablero;
    while (actual) {
        if (actual->getPosX() == x && actual->getPosY() == y) {
            return actual;
        }
        actual = actual->getSiguiente();
    }
    return nullptr;
}

bool esCeldaLibre(NodoSistema* tablero, int x, int y) {
    NodoSistema* nodo = buscarNodo(tablero, x, y);
    return nodo && nodo->getTanque() == nullptr;
}

// FUNCIÓN CORREGIDA: Buscar tanque en lista enlazada
pair<int, int> obtenerPosicionTanque(NodoSistema* tablero, Tanque* tanque) {
    NodoSistema* actual = tablero;
    while (actual) {
        if (actual->getTanque() == tanque) {
            return {actual->getPosX(), actual->getPosY()};
        }
        actual = actual->getSiguiente();
    }
    return {-1, -1}; // No encontrado
}

// Función para actualizar referencias de tanques después de clonar
void actualizarReferenciasTanques(NodoSistema* tablero, Tanque*& tanqueIA, Tanque*& tanqueJugador) {
    NodoSistema* actual = tablero;
    while (actual) {
        Tanque* tanque = actual->getTanque();
        if (tanque) {
            if (tanque->esIA()) {
                tanqueIA = tanque;
            } else {
                tanqueJugador = tanque;
            }
        }
        actual = actual->getSiguiente();
    }
}

void moverseSimulado(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    if (!tanque) return;

    NodoSistema* origen = nullptr;
    NodoSistema* destino = nullptr;
    NodoSistema* temp = tablero;

    while (temp) {
        if (temp->getTanque() == tanque) origen = temp;
        if (temp->getPosX() == posX && temp->getPosY() == posY) destino = temp;
        temp = temp->getSiguiente();
    }

    if (!origen || !destino || destino->getTanque()) return;

    origen->setTanque(nullptr);
    destino->setTanque(tanque);
}


void dispararSimulado(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    if (!tanque) return;

    NodoSistema* origen = nullptr;
    NodoSistema* objetivo = nullptr;
    NodoSistema* temp = tablero;

    while (temp) {
        if (temp->getTanque() == tanque) origen = temp;
        if (temp->getPosX() == posX && temp->getPosY() == posY) objetivo = temp;
        temp = temp->getSiguiente();
    }

    if (!origen || !objetivo) return;

    Tanque* blanco = objetivo->getTanque();
    if (!blanco) return;

    double probabilidad = tanque->getProbabilidadDeImpacto(origen->getTipoTerreno(), objetivo->getTipoTerreno());
    int numeroAleatorio = rand() % 100;
    if (numeroAleatorio < (probabilidad * 100)) {
        blanco->actualizarVida(tanque->getDanio());
    }
}


vector<EstadoJuego*> generarHijos(EstadoJuego* estado) {
    vector<EstadoJuego*> hijos;

    // Determinar el tanque que debe actuar
    Tanque* tanqueActual = estado->turnoIA ? estado->tanqueIA : estado->tanqueJugador;

    // Direcciones de movimiento (arriba, abajo, izquierda, derecha)
    vector<pair<int, int>> direcciones = {{0,1}, {0,-1}, {1,0}, {-1,0}};

    // === GENERAR MOVIMIENTOS POSIBLES ===
    for (auto& dir : direcciones) {
        // Clonar el tablero
        NodoSistema* clonTablero = clonarTablero(estado->tableroClonado);
        
        // Actualizar referencias a los tanques clonados
        Tanque* clonIA = nullptr;
        Tanque* clonJugador = nullptr;
        actualizarReferenciasTanques(clonTablero, clonIA, clonJugador);

        // Determinar el tanque que actúa
        Tanque* clonActual = estado->turnoIA ? clonIA : clonJugador;

        // Encontrar posición actual del tanque
        pair<int, int> pos = obtenerPosicionTanque(clonTablero, clonActual);
        if (pos.first == -1) continue; // Tanque no encontrado

        int nuevaX = pos.first + dir.first;
        int nuevaY = pos.second + dir.second;

        // Validar movimiento dentro de límites
        if (nuevaX >= 0 && nuevaX < 5 && nuevaY >= 0 && nuevaY < 5) {
            if (esCeldaLibre(clonTablero, nuevaX, nuevaY)) {
                // Usar la función de movimiento simulado
                moverseSimulado(clonActual, clonTablero, nuevaX, nuevaY);
                
                EstadoJuego* hijo = new EstadoJuego(clonTablero, clonIA, clonJugador, !estado->turnoIA);
                hijo->accionAplicada = MOVER;
                hijo->posAccionX = nuevaX;
                hijo->posAccionY = nuevaY;
                hijos.push_back(hijo);
                continue; // Evitar liberar memoria si el hijo es válido
            }
        }
        
        // Si llegamos aquí, el movimiento no fue válido, liberar memoria
        while (clonTablero) {
            NodoSistema* temp = clonTablero;
            clonTablero = clonTablero->getSiguiente();
            delete temp;
        }
    }

    // === GENERAR DISPAROS POSIBLES ===
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            NodoSistema* clonTablero = clonarTablero(estado->tableroClonado);
            
            Tanque* clonIA = nullptr;
            Tanque* clonJugador = nullptr;
            actualizarReferenciasTanques(clonTablero, clonIA, clonJugador);

            Tanque* clonActual = estado->turnoIA ? clonIA : clonJugador;

            // Verificar si puede disparar a esta posición (que haya un tanque enemigo)
            NodoSistema* objetivo = buscarNodo(clonTablero, x, y);
            if (objetivo && objetivo->getTanque() && objetivo->getTanque() != clonActual) {
                // Usar la función de disparo simulado
                dispararSimulado(clonActual, clonTablero, x, y);
                
                EstadoJuego* hijo = new EstadoJuego(clonTablero, clonIA, clonJugador, !estado->turnoIA);
                hijo->accionAplicada = DISPARAR;
                hijo->posAccionX = x;
                hijo->posAccionY = y;
                hijos.push_back(hijo);
                continue;
            }
            
            // Si no es un disparo válido, liberar memoria
            while (clonTablero) {
                NodoSistema* temp = clonTablero;
                clonTablero = clonTablero->getSiguiente();
                delete temp;
            }
        }
    }

    return hijos;
}

int evaluarEstado(EstadoJuego* estado) {
    int vidaIA = 0, vidaJugador = 0;
    int terrenoIA = 0, terrenoJugador = 0;
    int distanciaTotal = 0;

    NodoSistema* nodoIA = nullptr;
    NodoSistema* nodoJugador = nullptr;

    // Buscar tanques en el tablero clonado
    NodoSistema* actual = estado->tableroClonado;
    while (actual) {
        Tanque* t = actual->getTanque();
        if (t) {
            if (t->esIA()) {
                vidaIA = t->getVida();
                terrenoIA = actual->getTipoTerreno();
                nodoIA = actual;
            } else {
                vidaJugador = t->getVida();
                terrenoJugador = actual->getTipoTerreno();
                nodoJugador = actual;
            }
        }
        actual = actual->getSiguiente();
    }

    // Calcular distancia Manhattan
    if (nodoIA && nodoJugador) {
        int dx = abs(nodoIA->getPosX() - nodoJugador->getPosX());
        int dy = abs(nodoIA->getPosY() - nodoJugador->getPosY());
        distanciaTotal = dx + dy;
    }

    // Condiciones de victoria/derrota
    if (vidaIA <= 0) return -1000; // IA pierde
    if (vidaJugador <= 0) return 1000; // IA gana

    // Evaluación heurística
    return (vidaIA - vidaJugador) * 10 +
           (terrenoIA - terrenoJugador) * 3 -
           distanciaTotal * 2;
}

// === FUNCIÓN MINIMAX CON PODA ALFA-BETA CORREGIDA ===
int minimax(EstadoJuego* estado, int profundidad, int alpha, int beta, bool debug = false) {
    // Caso base: profundidad 0 o estado terminal
    if (profundidad == 0) {
        int val = evaluarEstado(estado);
        if (debug) {
            cout << "Evaluando estado hoja con valor: " << val << endl;
        }
        return val;
    }

    vector<EstadoJuego*> hijos = generarHijos(estado);
    
    // Si no hay movimientos posibles, evaluar estado actual
    if (hijos.empty()) {
        int val = evaluarEstado(estado);
        if (debug) {
            cout << "No hay movimientos disponibles, valor: " << val << endl;
        }
        return val;
    }

    int mejorValor;
    
    if (estado->turnoIA) {
        // Maximizar para la IA
        mejorValor = INT_MIN;
        for (EstadoJuego* hijo : hijos) {
            int eval = minimax(hijo, profundidad - 1, alpha, beta, debug);
            mejorValor = max(mejorValor, eval);
            alpha = max(alpha, eval);
            
            if (debug) {
                string accionStr = (hijo->accionAplicada == MOVER) ? "MOVER" : "DISPARAR";
                cout << "IA - " << accionStr << " a (" << hijo->posAccionX << "," 
                     << hijo->posAccionY << ") => Valor: " << eval << endl;
            }
            
            if (beta <= alpha) {
                if (debug) cout << "Poda alfa-beta" << endl;
                break; // Poda
            }
        }
    } else {
        // Minimizar para el jugador
        mejorValor = INT_MAX;
        for (EstadoJuego* hijo : hijos) {
            int eval = minimax(hijo, profundidad - 1, alpha, beta, debug);
            mejorValor = min(mejorValor, eval);
            beta = min(beta, eval);
            
            if (debug) {
                string accionStr = (hijo->accionAplicada == MOVER) ? "MOVER" : "DISPARAR";
                cout << "Jugador - " << accionStr << " a (" << hijo->posAccionX << "," 
                     << hijo->posAccionY << ") => Valor: " << eval << endl;
            }
            
            if (beta <= alpha) {
                if (debug) cout << "Poda alfa-beta" << endl;
                break; // Poda
            }
        }
    }

    // Liberar memoria de los hijos
    for (EstadoJuego* hijo : hijos) {
        delete hijo;
    }

    return mejorValor;
}

// Función para encontrar la mejor jugada
pair<Accion, pair<int, int>> encontrarMejorJugada(EstadoJuego* estadoActual, int profundidad) {
    vector<EstadoJuego*> hijos = generarHijos(estadoActual);
    
    if (hijos.empty()) {
        return {ESPERAR, {-1, -1}};
    }

    int mejorValor = INT_MIN;
    EstadoJuego* mejorHijo = nullptr;

    for (EstadoJuego* hijo : hijos) {
        int valor = minimax(hijo, profundidad - 1, INT_MIN, INT_MAX);
        
        cout << "Evaluando acción: " << (hijo->accionAplicada == MOVER ? "MOVER" : "DISPARAR")
             << " a (" << hijo->posAccionX << "," << hijo->posAccionY 
             << ") => Valor: " << valor << endl;
        
        if (valor > mejorValor) {
            mejorValor = valor;
            mejorHijo = hijo;
        }
    }

    Accion mejorAccion = mejorHijo->accionAplicada;
    pair<int, int> mejorPosicion = {mejorHijo->posAccionX, mejorHijo->posAccionY};

    // Liberar memoria
    for (EstadoJuego* hijo : hijos) {
        delete hijo;
    }

    return {mejorAccion, mejorPosicion};
}

// ========== FUNCIONES PARA EJECUTAR ACCIONES REALES ==========
// Declaraciones de las funciones que ya tienes implementadas
void moverseSimulado(Tanque* tanque, NodoSistema* tablero, int posX, int posY);
void dispararSimulado(Tanque* tanque, NodoSistema* tablero, int posX, int posY);

bool ejecutarMovimiento(NodoSistema* tableroReal, Tanque* tanque, int nuevaX, int nuevaY) {
    // Validar que la nueva posición esté libre
    if (!esCeldaLibre(tableroReal, nuevaX, nuevaY)) return false;

    // Usar tu función de movimiento
    moverseSimulado(tanque, tableroReal, nuevaX, nuevaY);

    cout << "Tanque movido a (" << nuevaX << "," << nuevaY << ")" << endl;
    return true;
}

bool ejecutarDisparo(NodoSistema* tableroReal, Tanque* atacante, int objetivoX, int objetivoY) {
    // Encontrar el nodo objetivo
    NodoSistema* nodoObjetivo = buscarNodo(tableroReal, objetivoX, objetivoY);
    if (!nodoObjetivo || !nodoObjetivo->getTanque()) return false;

    Tanque* tanqueObjetivo = nodoObjetivo->getTanque();
    
    // No puede atacarse a sí mismo
    if (tanqueObjetivo == atacante) return false;

    // Usar tu función de disparo
    dispararSimulado(atacante, tableroReal, objetivoX, objetivoY);

    cout << "Disparo realizado a (" << objetivoX << "," << objetivoY << ")" << endl;
    return true;
}

// Función principal para que la IA tome una decisión y la ejecute
bool turnoIA(NodoSistema* tableroReal, Tanque* tanqueIA, Tanque* tanqueJugador, int profundidad = 6) {
    cout << "\n=== TURNO DE LA IA ===" << endl;
    
    // Crear estado actual
    EstadoJuego* estadoActual = new EstadoJuego(tableroReal, tanqueIA, tanqueJugador, true);
    
    // Encontrar la mejor jugada
    auto mejorJugada = encontrarMejorJugada(estadoActual, profundidad);
    
    Accion accion = mejorJugada.first;
    int x = mejorJugada.second.first;
    int y = mejorJugada.second.second;
    
    bool exito = false;
    
    // EJECUTAR LA ACCIÓN EN EL TABLERO REAL
    if (accion == MOVER) {
        cout << "IA decide MOVERSE a (" << x << "," << y << ")" << endl;
        exito = ejecutarMovimiento(tableroReal, tanqueIA, x, y);
    } 
    else if (accion == DISPARAR) {
        cout << "IA decide DISPARAR a (" << x << "," << y << ")" << endl;
        exito = ejecutarDisparo(tableroReal, tanqueIA, x, y);
    }
    else {
        cout << "IA decide ESPERAR" << endl;
        exito = true; // Esperar siempre es válido
    }
    
    delete estadoActual;
    
    if (!exito) {
        cout << "Error: No se pudo ejecutar la acción de la IA" << endl;
    }
    
    return exito;
}

void recorrerTablero(NodoSistema* head) {
    NodoSistema* temp = head;
    cout << "--- Tablero generado ---" << endl;
    while (temp != nullptr) {
        cout << "Nodo ID: " << temp->getIdNodo()
             << ", Posicion: (" << temp->getPosX() << ", " << temp->getPosY() << ")"
             << ", Tipo de Terreno: " << temp->getTipoTerreno() << endl;
        temp = temp->getSiguiente();
    }
}

// Función que agrega un nodo al final de la lista doblemente enlazada
NodoSistema* agregarPosicion(NodoSistema* head, NodoSistema* nuevoNodo) {
    if (head == nullptr) {
        return nuevoNodo;
    }

    NodoSistema* temp = head;
    while (temp->getSiguiente() != nullptr) {
        temp = temp->getSiguiente();
    }

    temp->setSiguiente(nuevoNodo);
    nuevoNodo->setAnterior(temp);
    return head;
}

// Función que genera el tablero 5x5 con diferentes tipos de terreno
NodoSistema* crearTablero() {
    NodoSistema* listaPosiciones = nullptr;
    int contador = 0;

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int tipoTerreno;
            if (i == 0 || i == 4) {
                tipoTerreno = 1;
            } else if (i == 1 || i == 3) {
                tipoTerreno = 2;
            } else {
                tipoTerreno = 3;
            }

            NodoSistema* nuevoNodo = new NodoSistema(contador, j, i, tipoTerreno);
            listaPosiciones = agregarPosicion(listaPosiciones, nuevoNodo);
            contador++;
        }
    }

    return listaPosiciones;
}

// Función que dispara un tanque a una posición específica
void disparar(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    cout << "Tanque ID: " << tanque->getIdTanque() << " disparando!" << endl;

    // Buscar la posición del tanque que dispara
    NodoSistema* tanqueDisparando = nullptr;
    NodoSistema* temp = tablero;
    while (temp != nullptr) {
        if (temp->getTanque() == tanque) {
            tanqueDisparando = temp;
            break;
        }
        temp = temp->getSiguiente();
    }

    if (tanqueDisparando == nullptr) {
        cout << "Error: El tanque que dispara no se encuentra en el tablero." << endl;
        return;
    }

    cout << "Tanque que dispara se encuentra en la posicion: (" 
         << tanqueDisparando->getPosX() << ", " << tanqueDisparando->getPosY() << ")" << endl;

    // Buscar el nodo objetivo
    temp = tablero;
    srand(time(0));
    while (temp != nullptr) {
        if (temp->getPosX() == posX && temp->getPosY() == posY) {
            Tanque* tanqueEnPosicion = temp->getTanque();

            int terrenoInicio = tanqueDisparando->getTipoTerreno();
            int terrenoFinal = temp->getTipoTerreno();
            double probabilidad = tanque->getProbabilidadDeImpacto(terrenoInicio, terrenoFinal);

            cout << "Probabilidad de acierto: " << probabilidad * 100 << "%" << endl;

            int numeroAleatorio = rand() % 100;
            cout << "Numero aleatorio: " << numeroAleatorio << endl;

            if (numeroAleatorio < (probabilidad * 100)) {
                if (tanqueEnPosicion != nullptr) {
                    tanqueEnPosicion->actualizarVida(tanque->getDanio());
                    cout << "Tanque ID: " << tanqueEnPosicion->getIdTanque() << " ha sido alcanzado!" << endl;
                    cout << "Vida restante: " << tanqueEnPosicion->getVida() << endl;
                    // ...dentro de disparar, cuando el disparo acierta...
                    explosiones.push_back({posX, posY, 5.0f, TANQUE});
                    std::cout << "Explosión agregada en (" << posX << ", " << posY << ")" << std::endl;
                    ultimaExplosion = TANQUE;
                } else {
                    cout << "¡Fallaste! No hay tanque en la posicion (" << posX << ", " << posY << ")." << endl;
                    explosiones.push_back({posX, posY, 5.0f, TERRENO});
                    ultimaExplosion = TERRENO;
                }
            } else {
                cout << "¡Fallaste! El disparo no tuvo exito." << endl;
                ultimaExplosion = NINGUNA;
            }
            break;
        }
        temp = temp->getSiguiente();
    }
}

// Función que mueve un tanque a una posición específica
void moverse(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    if (tanque == nullptr) {
        cout << "Error: el tanque es nulo." << endl;
        return;
    }

    cout << "Tanque ID: " << tanque->getIdTanque() << " intentando moverse a (" << posX << ", " << posY << ")" << endl;

    NodoSistema* destino = nullptr;
    NodoSistema* origen = nullptr;

    NodoSistema* temp = tablero;
    while (temp != nullptr) {
        if (temp->getTanque() == tanque) {
            origen = temp;
        }
        if (temp->getPosX() == posX && temp->getPosY() == posY) {
            destino = temp;
        }
        temp = temp->getSiguiente();
    }

    if (!destino) {
        cout << "Error: la posición destino no existe en el tablero." << endl;
        return;
    }

    if (destino->getTanque() != nullptr) {
        cout << "Error: ya hay un tanque en la posición destino." << endl;
        return;
    }

    if (!origen) {
        cout << "Advertencia: el tanque no estaba posicionado previamente." << endl;
    } else {
        origen->setTanque(nullptr);
    }

    destino->setTanque(tanque);

    cout << "Tanque movido exitosamente a la posición: (" << posX << ", " << posY << ")" << endl;
}




bool mostrarMenuSeleccionTanquesJugador(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*>& tanquesJugador,
    NodoSistema* tableroPosiciones
) {
    std::vector<std::string> opciones = {
        "Tanque Ligero",
        "Tanque Mediano",
        "Tanque Pesado"
    };
    int seleccion = 0;
    int idTanque = 1;
    std::vector<Tanque*> seleccionados;

    enum Estado { SELECCION_TANQUE, INGRESO_COORDENADAS };
    Estado estado = SELECCION_TANQUE;

    Tanque* tanqueParaColocar = nullptr;
    std::string inputX, inputY;
    bool escribiendoY = false;

    const int filas = 5, columnas = 5;

    auto centrarTexto = [](sf::Text& texto) {
        sf::FloatRect bounds = texto.getLocalBounds();
        texto.setOrigin(bounds.left + bounds.width / 2.f,
                        bounds.top + bounds.height / 2.f);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                for (auto t : seleccionados) delete t;
                delete tanqueParaColocar;
                window.close();
                return false;
            }
            else if (event.type == sf::Event::Resized) {
                // Actualizar la vista para el nuevo tamaño
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }

            if (estado == SELECCION_TANQUE && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    if (seleccionados.size() == 3) {
                        for (int i = (int)seleccionados.size() - 1; i >= 0; --i)
                            tanquesJugador.push(seleccionados[i]);
                        seleccionados.clear();
                        return true;
                    }
                    switch (seleccion) {
                        case 0: tanqueParaColocar = new TanqueLigero(idTanque++); break;
                        case 1: tanqueParaColocar = new TanqueMediano(idTanque++); break;
                        case 2: tanqueParaColocar = new TanquePesado(idTanque++); break;
                    }
                    inputX.clear();
                    inputY.clear();
                    escribiendoY = false;
                    estado = INGRESO_COORDENADAS;
                } else if (event.key.code == sf::Keyboard::Backspace) {
                    if (!seleccionados.empty()) {
                        Tanque* tanqueAEliminar = seleccionados.back();
                        if (tanqueAEliminar != nullptr) {
                            NodoSistema* temp = tableroPosiciones;
                            bool encontrado = false;
                            while (temp) {
                                if (temp->getTanque() == tanqueAEliminar) {
                                    temp->setTanque(nullptr);
                                    encontrado = true;
                                    break;
                                }
                                temp = temp->getSiguiente();
                            }

                            if (encontrado) {
                                std::cout << "Tanque encontrado en tablero, eliminando referencia (sin delete).\n";
                                seleccionados.pop_back();
                            } else {
                                std::cout << "ERROR: Tanque no encontrado en tablero al intentar eliminar.\n";
                            }
                        } else {
                            std::cout << "ERROR: Tanque a eliminar es nullptr.\n";
                        }
                    } else {
                        std::cout << "ERROR: No hay tanques para eliminar.\n";
                    }
                } else if (event.key.code == sf::Keyboard::Escape) {
                    for (auto t : seleccionados) delete t;
                    seleccionados.clear();
                    delete tanqueParaColocar;
                    tanqueParaColocar = nullptr;
                    return false;
                }
            } else if (estado == INGRESO_COORDENADAS) {
                if (event.type == sf::Event::TextEntered) {
                    char c = static_cast<char>(event.text.unicode);
                    if (isdigit(c)) {
                        int valor = c - '0';
                        if (valor >= 0 && valor <= 4) {
                            if (!escribiendoY && inputX.empty()) {
                                inputX += c;
                            } else if (escribiendoY && inputY.empty()) {
                                inputY += c;
                            }
                        }
                    } else if (c == ' ' && !inputX.empty() && !escribiendoY) {
                        escribiendoY = true;
                    }
                } else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        if (escribiendoY) {
                            if (!inputY.empty()) inputY.pop_back();
                        } else {
                            if (!inputX.empty()) inputX.pop_back();
                        }
                    } else if (event.key.code == sf::Keyboard::Left) {
                        escribiendoY = false;
                    } else if (event.key.code == sf::Keyboard::Right) {
                        escribiendoY = true;
                    } else if (event.key.code == sf::Keyboard::Space) {
                        escribiendoY = !escribiendoY;
                    } else if (event.key.code == sf::Keyboard::Escape) {
                        delete tanqueParaColocar;
                        tanqueParaColocar = nullptr;
                        estado = SELECCION_TANQUE;
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        if (!inputY.empty() && !inputX.empty()) {
                            int y = std::stoi(inputY);
                            int x = std::stoi(inputX);
                            NodoSistema* temp = tableroPosiciones;
                            NodoSistema* destino = nullptr;
                            while (temp) {
                                if (temp->getPosX() == x && temp->getPosY() == y) {
                                    destino = temp;
                                    break;
                                }
                                temp = temp->getSiguiente();
                            }
                            if (destino && destino->getTanque() == nullptr && y < 2) {
                                destino->setTanque(tanqueParaColocar);
                                seleccionados.push_back(tanqueParaColocar);
                                tanqueParaColocar = nullptr;
                                estado = SELECCION_TANQUE;
                            } else {
                                inputX.clear();
                                inputY.clear();
                                escribiendoY = false;
                            }
                        }
                    }
                }
            }
        }

        // Obtener tamaño actual ventana
        sf::Vector2u size = window.getSize();
        float width = static_cast<float>(size.x);
        float height = static_cast<float>(size.y);

        window.clear(sf::Color::Black);

        // Tamaños dinámicos basados en ventana
        unsigned int tamTitulo = static_cast<unsigned int>(height / 15.f);
        unsigned int tamOpciones = static_cast<unsigned int>(height / 30.f);
        unsigned int tamInfo = static_cast<unsigned int>(height / 40.f);

        // Posiciones y márgenes dinámicos
        float marginX = width * 0.1f;
        float marginY = height * 0.08f;

        // --- Dibujo titulo ---
        sf::Text titulo("Selecciona tus tanques", font, tamTitulo);
        titulo.setFillColor(sf::Color(110, 180, 100));
        // Centrar horizontal
        sf::FloatRect boundsTitulo = titulo.getLocalBounds();
        titulo.setOrigin(boundsTitulo.left + boundsTitulo.width / 2.f,
                         boundsTitulo.top + boundsTitulo.height / 2.f);
        titulo.setPosition(width / 2.f, marginY);
        window.draw(titulo);

        // --- Dibujo Volver ---
        sf::Text volver("Volver (ESC)", font, tamInfo);
        volver.setFillColor(sf::Color(180, 180, 180));
        volver.setPosition(marginX * 0.3f, marginY * 0.3f);
        window.draw(volver);

        if (estado == SELECCION_TANQUE) {
            // Opciones de tanques
            for (int i = 0; i < (int)opciones.size(); ++i) {
                sf::Text txt(opciones[i], font, tamOpciones);

                // Posición vertical dinámica
                float posY = marginY * 2.f + i * (tamOpciones * 1.8f);
                txt.setPosition(marginX, posY);

                if (i == seleccion) {
                    sf::FloatRect bounds = txt.getLocalBounds();

                    // Márgenes (padding) fijos o relativos al tamaño de fuente
                    float paddingX = tamOpciones * 0.5f;  // espacio horizontal a cada lado
                    float paddingY = tamOpciones * 0.3f;  // espacio vertical arriba y abajo

                    sf::RectangleShape highlight(sf::Vector2f(bounds.width + 2 * paddingX, bounds.height + 2 * paddingY));
                    highlight.setFillColor(sf::Color(110, 180, 100));

                    // Posición ajustada para que el rectángulo quede centrado verticalmente y con margen izquierdo
                    highlight.setPosition(txt.getPosition().x - paddingX + bounds.left, txt.getPosition().y - paddingY + bounds.top);

                    window.draw(highlight);

                    txt.setFillColor(sf::Color::Black);
                } else {
                    txt.setFillColor(sf::Color::White);
                }
                window.draw(txt);
            }
        } else {
            sf::Text info("Ingresa coordenadas: X (columna) Y (fila) - Solo filas 0 y 1", font, tamOpciones);
            info.setFillColor(sf::Color::Green);
            info.setPosition(marginX, marginY * 2.2f);
            window.draw(info);

            std::string coordenadas = "X: " + inputX + " Y: " + inputY;
            if (escribiendoY) coordenadas += "  <--- editando Y";
            else coordenadas += "  <--- editando X";

            sf::Text entrada(coordenadas, font, tamOpciones);
            entrada.setFillColor(sf::Color::White);
            entrada.setPosition(marginX, marginY * 2.8f);
            window.draw(entrada);

            if (tanqueParaColocar) {
                sf::Text stats(
                    "Vida: " + std::to_string(tanqueParaColocar->getVida()) +
                    "  Danio: " + std::to_string(tanqueParaColocar->getDanio()) +
                    "  Mov: " + std::to_string(tanqueParaColocar->getMovimientoBase()),
                    font, tamInfo);
                stats.setFillColor(sf::Color::Cyan);
                stats.setPosition(marginX, marginY * 3.4f);
                window.draw(stats);
            }
        }

        // Contador de tanques seleccionados
        sf::Text count("Tanques elegidos: " + std::to_string(seleccionados.size()), font, tamOpciones);
        count.setFillColor(sf::Color::White);
        count.setPosition(marginX, marginY * 4.5f);
        window.draw(count);

        // --- Dibujo tablero ---
        // Ajustar cellSize según tamaño ventana
        float maxTableroWidth = width * 0.8f; // que el tablero ocupe max 80% ancho
        float maxTableroHeight = height * 0.5f; // max 50% alto

        float cellSizeX = maxTableroWidth / columnas;
        float cellSizeY = maxTableroHeight / filas;

        float cellSize = std::min(cellSizeX, cellSizeY);

        float tableroAncho = cellSize * columnas;
        float tableroAlto = cellSize * filas;

        // Centrar tablero horizontalmente, posicion vertical abajo de contador
        float offsetX = (width - tableroAncho) / 2.f;
        float offsetY = marginY * 5.5f;

        NodoSistema* cur = tableroPosiciones;
        while (cur) {
            sf::RectangleShape sq({cellSize, cellSize});
            sq.setPosition(offsetX + cur->getPosX() * cellSize,
                           offsetY + cur->getPosY() * cellSize);
            sq.setFillColor(cur->getTanque() ? sf::Color::Red : sf::Color::White);
            sq.setOutlineColor(sf::Color::Black);
            sq.setOutlineThickness(cellSize * 0.05f); // ajustar grosor borde
            window.draw(sq);
            cur = cur->getSiguiente();
        }

        // --- Mostrar controles disponibles ---
        sf::Text controles("", font, tamInfo * 0.9f);
        controles.setFillColor(sf::Color(160, 160, 160));

        std::string textoControles;
        if (estado == SELECCION_TANQUE) {
            textoControles = "Controles: UP/DOWN para moverse, ENTER para seleccionar, BACKSPACE para eliminar, ESC para volver";
        } else {
            textoControles = "Controles: numeros 0-4 para X/Y, SPACE o flechas para cambiar campo, ENTER para confirmar, ESC para cancelar";
        }
        controles.setString(textoControles);
        controles.setPosition(marginX * 0.3f, height - marginY * 0.6f);
        window.draw(controles);

        // --- Mostrar estado actual (accion que se esta haciendo) ---
        sf::Text estadoActual("", font, tamInfo);
        estadoActual.setFillColor(sf::Color::Yellow);

        std::string textoEstado;
        if (estado == SELECCION_TANQUE) {
            textoEstado = "Estado: seleccionando tipo de tanque";
        } else {
            textoEstado = "Estado: ingresando coordenadas";
        }
        estadoActual.setString(textoEstado);
        estadoActual.setPosition(marginX * 0.3f, height - marginY * 1.2f);
        window.draw(estadoActual);


        window.display();
    }

    return false;
}


// Función que selecciona tanques para la IA
void seleccionarTanquesIA(
    std::stack<Tanque*>& tanquesIA,
    NodoSistema* tableroPosiciones
) {
    std::vector<std::string> tiposTanque = {"Ligero", "Mediano", "Pesado"};
    int idTanque = 100;

    
    const int columnas = 5;
    const int filas = 5;

    std::vector<NodoSistema*> nodosDisponibles;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    NodoSistema* temp = tableroPosiciones;
    while (temp != nullptr) {
        int y = temp->getPosY();
        if (temp->getTanque() == nullptr && (y == filas - 1 || y == filas - 2)) {
            //cout<< "Nodo disponible: (" << temp->getPosX() << ", " << temp->getPosY() << ")" << endl;
            nodosDisponibles.push_back(temp);
        }
        temp = temp->getSiguiente();
    }

    if (nodosDisponibles.size() < 3) {
        std::cerr << "No hay suficientes posiciones libres en las filas finales para la IA.\n";
        return;
    }

    for (int i = 0; i < 3; ++i) {
        int tipo = rand() % 3;
        Tanque* nuevoTanque = nullptr;
        switch (tipo) {
            case 0: nuevoTanque = new TanqueLigero(idTanque++); break;
            case 1: nuevoTanque = new TanqueMediano(idTanque++); break;
            case 2: nuevoTanque = new TanquePesado(idTanque++); break;
        }

        int indiceNodo = rand() % nodosDisponibles.size();

        //cout<<"Nodo seleccionado: " << nodosDisponibles[indiceNodo]->getPosX() << ", " << nodosDisponibles[indiceNodo]->getPosY() << endl;
        NodoSistema* nodoSeleccionado = nodosDisponibles[indiceNodo];
        nodoSeleccionado->setTanque(nuevoTanque);
        tanquesIA.push(nuevoTanque);
        nodosDisponibles.erase(nodosDisponibles.begin() + indiceNodo);
    }
}


// Función que despliega el tablero en la ventana
void desplegarTablero(
    sf::RenderWindow& window,
    sf::Font& font,
    int filas,
    int columnas,
    int cellSize,
    NodoSistema* tableroPosiciones,
    sf::Texture& texturaTerreno1,
    sf::Texture& texturaTerreno2,
    sf::Texture& texturaTerreno3,
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA,
    sf::Texture& texturaExplosionTerreno,
    sf::Texture& texturaExplosionTanque
) {

    // Calcula el tiempo real entre frames si puedes, aquí fijo a 1/60s (~60 FPS)
    float deltaTime = 1.0f / 60.0f;
    for (auto& ex : explosiones) {
        ex.tiempoRestante -= deltaTime;
    }
    explosiones.erase(
        std::remove_if(explosiones.begin(), explosiones.end(),
                    [](const Explosion& e){ return e.tiempoRestante <= 0; }),
        explosiones.end()
    );
    const int offsetTableroX = 50;
    const int offsetTableroY = 90;

    // Dibujar coordenadas X (col)
    for (int col = 0; col < columnas; col++) {
        sf::Text text(std::to_string(col), font, 18);
        text.setFillColor(sf::Color::White);
        sf::FloatRect bounds = text.getLocalBounds();

        float posX = col * cellSize + offsetTableroX + (cellSize - bounds.width) / 2.0f - bounds.left;
        float posY = offsetTableroY - 30;  // encima del tablero
        text.setPosition(posX, posY);
        window.draw(text);
    }

    // Dibujar coordenadas Y (filas)
    for (int row = 0; row < filas; row++) {
        sf::Text text(std::to_string(row), font, 18);
        text.setFillColor(sf::Color::White);
        sf::FloatRect bounds = text.getLocalBounds();

        float posX = offsetTableroX - 25;
        float posY = row * cellSize + offsetTableroY + (cellSize - bounds.height) / 2.0f - bounds.top;
        text.setPosition(posX, posY);
        window.draw(text);
    }

    // Recorrer el tablero y dibujar terrenos + tanques
    NodoSistema* actual = tableroPosiciones;
    while (actual != nullptr) {
        // Terreno
        sf::Sprite sprite;
        switch (actual->getTipoTerreno()) {
            case 1: sprite.setTexture(texturaTerreno1); break;
            case 2: sprite.setTexture(texturaTerreno2); break;
            case 3: sprite.setTexture(texturaTerreno3); break;
            default: sprite.setColor(sf::Color::Red); break;
        }

        float x = actual->getPosX() * cellSize + offsetTableroX;
        float y = actual->getPosY() * cellSize + offsetTableroY;
        sprite.setPosition(x, y);
        if (sprite.getTexture()) {
            sprite.setScale(
                (float)cellSize / sprite.getTexture()->getSize().x,
                (float)cellSize / sprite.getTexture()->getSize().y
            );
        }
        window.draw(sprite);
        

        // Tanques
        if (actual->getTanque() != nullptr && actual->getTanque()->getVida()>0) {
            sf::Sprite spriteTanque;
            Tanque* t = actual->getTanque();
            bool esIA = t->getIdTanque() >= 100;
            int mov = t->getMovimientoBase();

            //std::cout << "Tanque ID: " << t->getIdTanque() << ", mov: " << mov << std::endl;

            sf::Texture& tex = esIA ? (mov == 6 ? texturaTanqueLigeroIA : 
                            (mov == 4 ? texturaTanqueMedianoIA : texturaTanquePesadoIA)) 
                        : (mov == 6 ? texturaTanqueLigeroJugador :
                            (mov == 4 ? texturaTanqueMedianoJugador : texturaTanquePesadoJugador));

            if (tex.getSize().x == 0 || tex.getSize().y == 0) {
                std::cout << "Error: textura vacía para tanque ID: " << t->getIdTanque() << std::endl;
            } else {
                spriteTanque.setTexture(tex);
            }

            //cout<<"Llega a la condicional de mov"<<endl;
            if (mov == 6)
                spriteTanque.setTexture(esIA ? texturaTanqueLigeroIA : texturaTanqueLigeroJugador);
            else if (mov == 4)
                spriteTanque.setTexture(esIA ? texturaTanqueMedianoIA : texturaTanqueMedianoJugador);
            else if (mov == 2)
                spriteTanque.setTexture(esIA ? texturaTanquePesadoIA : texturaTanquePesadoJugador);

            if (spriteTanque.getTexture()) {
                float scaleX = 0.5f * (float)cellSize / spriteTanque.getTexture()->getSize().x;
                float scaleY = 0.5f * (float)cellSize / spriteTanque.getTexture()->getSize().y;
                spriteTanque.setScale(scaleX, scaleY);

                if (!esIA) {
                    auto ts = spriteTanque.getTexture()->getSize();
                    spriteTanque.setOrigin(ts.x / 2.f, ts.y / 2.f);
                    spriteTanque.setPosition(x + cellSize / 2.f, y + cellSize / 2.f);
                    spriteTanque.setRotation(180.f);
                } else {
                    float offsetX = (cellSize - spriteTanque.getTexture()->getSize().x * scaleX) / 2.f;
                    float offsetY = (cellSize - spriteTanque.getTexture()->getSize().y * scaleY) / 2.f;
                    spriteTanque.setPosition(x + offsetX, y + offsetY);
                }

                window.draw(spriteTanque);
            } else {
                std::cout << "Error: spriteTanque no tiene textura asignada para tanque ID: " << t->getIdTanque() << std::endl;
            }


            if (!spriteTanque.getTexture()) {
                std::cout << "Error: spriteTanque no tiene textura asignada" << std::endl;
            }

        }

        actual = actual->getSiguiente();
    }

    // Ahora dibuja todas las explosiones activas
    /*for (const Explosion& ex : explosiones) {
        sf::Sprite spriteExplosion;
        const sf::Texture* tex = nullptr;
        if (ex.tipo == TANQUE) {
            spriteExplosion.setTexture(texturaExplosionTanque);
            tex = &texturaExplosionTanque;
        } else {
            spriteExplosion.setTexture(texturaExplosionTerreno);
            tex = &texturaExplosionTerreno;
        }
        spriteExplosion.setPosition(
            ex.x * cellSize + offsetTableroX,
            ex.y * cellSize + offsetTableroY
        );
        // Ajusta el scale según la textura correcta
        spriteExplosion.setScale(
            3.5f * (float)cellSize / tex->getSize().x,
            3.5f * (float)cellSize / tex->getSize().y
        );
        window.draw(spriteExplosion);
    }*/

}


int mostrarMenuDificultad(sf::RenderWindow& window, sf::Font& font) {
    std::vector<std::string> opciones = {"Facil", "Media", "Dificil"};
    int seleccion = 0;

    auto centrarTexto = [](sf::Text& texto) {
        sf::FloatRect bounds = texto.getLocalBounds();
        texto.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return -2;  // Salida completa
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    return seleccion;  // 0 = Facil, 1 = Media, 2 = Dificil
                } else if (event.key.code == sf::Keyboard::Escape) {
                    return -1;  // Volver al menú principal
                }
            } else if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
        }

        window.clear(sf::Color::Black);

        sf::Vector2u size = window.getSize();
        float centerX = size.x / 2.f;
        float totalHeight = static_cast<float>(size.y);

        // Tamaños de texto dinámicos
        unsigned int tamTitulo = totalHeight / 12;
        unsigned int tamOpciones = totalHeight / 20;

        // Título
        sf::Text titulo("Selecciona la Dificultad", font, tamTitulo);
        titulo.setFillColor(sf::Color(110, 180, 100));
        titulo.setStyle(sf::Text::Bold);
        centrarTexto(titulo);
        titulo.setPosition(centerX, totalHeight * 0.15f);
        window.draw(titulo);

        // Opciones espaciadas dinámicamente debajo del título
        float inicioOpcionesY = totalHeight * 0.35f;
        float espacio = totalHeight * 0.12f;

        for (int i = 0; i < (int)opciones.size(); ++i) {
            sf::Text texto(opciones[i], font, tamOpciones);
            texto.setStyle(sf::Text::Bold);
            centrarTexto(texto);
            texto.setPosition(centerX, inicioOpcionesY + i * espacio);

            if (i == seleccion) {
                sf::FloatRect global = texto.getGlobalBounds();
                sf::RectangleShape highlight(sf::Vector2f(global.width + 40.f, global.height + 30.f));
                highlight.setFillColor(sf::Color(110, 180, 100));
                highlight.setOrigin(highlight.getSize().x / 2.f, highlight.getSize().y / 2.f);
                highlight.setPosition(texto.getPosition());
                window.draw(highlight);
                texto.setFillColor(sf::Color::Black);
            } else {
                texto.setFillColor(sf::Color::White);
            }

            window.draw(texto);
        }

        window.display();
    }

    return -2;
}



void mostrarPantallaInicio(sf::RenderWindow& window, sf::Font& font) {
    sf::Text titulo("FERRUM BELLUM", font);
    titulo.setFillColor(sf::Color(110, 180, 100));
    titulo.setStyle(sf::Text::Bold);

    sf::Text mensaje("Presione cualquier tecla para comenzar su batalla...", font);
    mensaje.setFillColor(sf::Color::White);

    sf::Clock clock;
    bool mostrarMensaje = true;

    auto centrarTexto = [](sf::Text& texto) {
        sf::FloatRect bounds = texto.getLocalBounds();
        texto.setOrigin(bounds.left + bounds.width / 2.f,
                        bounds.top + bounds.height / 2.f);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            } else if (event.type == sf::Event::KeyPressed) {
                sf::Vector2u size = window.getSize();
                window.setView(sf::View(sf::FloatRect(0, 0, size.x, size.y)));
                return;
            } else if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
        }

        if (clock.getElapsedTime().asSeconds() > 0.5f) {
            mostrarMensaje = !mostrarMensaje;
            clock.restart();
        }

        sf::Vector2u size = window.getSize();
        float centerX = size.x / 2.f;
        float centerY = size.y / 2.f;

        // Tamaños dinámicos
        unsigned int tamTitulo = size.y / 9;
        unsigned int tamMensaje = size.y / 32;

        titulo.setCharacterSize(tamTitulo);
        centrarTexto(titulo);
        titulo.setPosition(centerX, centerY - tamTitulo);

        mensaje.setCharacterSize(tamMensaje);
        centrarTexto(mensaje);
        mensaje.setPosition(centerX, size.y - tamMensaje * 2.5f);

        window.clear(sf::Color::Black);
        window.draw(titulo);
        if (mostrarMensaje)
            window.draw(mensaje);
        window.display();
    }
}



int mostrarMenuPrincipal(sf::RenderWindow& window, sf::Font& font) {
    std::vector<std::string> opciones = {"Jugar", "Salir"};
    int seleccion = 0;

    auto centrarTexto = [](sf::Text& texto) {
        sf::FloatRect bounds = texto.getLocalBounds();
        texto.setOrigin(bounds.left + bounds.width / 2.f,
                        bounds.top + bounds.height / 2.f);
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 1;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    return seleccion;
                }
            } else if (event.type == sf::Event::Resized) {
                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
        }

        sf::Vector2u size = window.getSize();
        float centerX = size.x / 2.f;
        float centerY = size.y / 2.f;

        // Tamaños adaptativos
        unsigned int tamTitulo = size.y / 9;
        unsigned int tamOpciones = size.y / 18;
        float espacio = tamOpciones * 2.f;

        // Título
        sf::Text titulo("FERRUM BELLUM", font, tamTitulo);
        titulo.setFillColor(sf::Color(110, 180, 100));
        titulo.setStyle(sf::Text::Bold);
        centrarTexto(titulo);
        titulo.setPosition(centerX, centerY - espacio * 2);
        
        window.clear(sf::Color::Black);
        window.draw(titulo);

        // Opciones
        for (int i = 0; i < (int)opciones.size(); ++i) {
            sf::Text texto(opciones[i], font, tamOpciones);
            texto.setStyle(sf::Text::Bold);
            centrarTexto(texto);
            texto.setPosition(centerX, centerY + i * espacio);

            if (i == seleccion) {
                sf::FloatRect global = texto.getGlobalBounds();
                sf::RectangleShape highlight(sf::Vector2f(global.width + 40.f, global.height + 20.f));
                highlight.setFillColor(sf::Color(110, 180, 100));
                highlight.setOrigin(highlight.getSize().x / 2.f, highlight.getSize().y / 2.f);
                highlight.setPosition(texto.getPosition());
                window.draw(highlight);
                texto.setFillColor(sf::Color::Black);
            } else {
                texto.setFillColor(sf::Color::White);
            }

            window.draw(texto);
        }

        window.display();
    }

    return 1;
}

// Estructura para almacenar una acción planificada
struct AccionPlanificada {
    Tanque* tanque;
    std::string tipoAccion; // "Moverse" o "Atacar"
    int coordX;
    int coordY;
    bool esValida;
    
    AccionPlanificada() : tanque(nullptr), tipoAccion(""), coordX(-1), coordY(-1), esValida(false) {}
    AccionPlanificada(Tanque* t, std::string tipo, int x, int y) 
        : tanque(t), tipoAccion(tipo), coordX(x), coordY(y), esValida(true) {}
};

// Función para obtener la acción de la IA (modificada para retornar en lugar de ejecutar)
AccionPlanificada obtenerAccionIA(
    std::stack<Tanque*>& tanquesIA,
    NodoSistema* tablero,
    int filas,
    int columnas
) {
    // Obtener tanques vivos
    std::stack<Tanque*> copia = tanquesIA;
    std::vector<Tanque*> tanquesDisponibles;

    while (!copia.empty()) {
        if (copia.top()->getVida() > 0)
            tanquesDisponibles.push_back(copia.top());
        copia.pop();
    }

    if (tanquesDisponibles.empty()) return AccionPlanificada();

    // Elegir un tanque aleatorio
    Tanque* tanqueIA = tanquesDisponibles[rand() % tanquesDisponibles.size()];

    // Encontrar el tanque del jugador
    Tanque* tanqueJugador = nullptr;
    NodoSistema* temp = tablero;
    while (temp) {
        Tanque* t = temp->getTanque();
        if (t && !t->esIA() && t->getVida() > 0) {
            tanqueJugador = t;
            break;
        }
        temp = temp->getSiguiente();
    }

    if (!tanqueJugador) {
        std::cout << "No hay tanques enemigos vivos" << std::endl;
        return AccionPlanificada();
    }

    // Crear estado y encontrar mejor jugada
    EstadoJuego* estadoActual = new EstadoJuego(tablero, tanqueIA, tanqueJugador, true);
    auto mejorJugada = encontrarMejorJugada(estadoActual, 6);
    
    Accion mejorAccion = mejorJugada.first;
    int mejorX = mejorJugada.second.first;
    int mejorY = mejorJugada.second.second;

    // Retornar la acción planificada en lugar de ejecutarla
    if (mejorAccion == MOVER) {
        std::cout << "IA planifica MOVERSE a (" << mejorX << "," << mejorY << ")" << std::endl;
        return AccionPlanificada(tanqueIA, "Moverse", mejorX, mejorY);
    } 
    else if (mejorAccion == DISPARAR) {
        std::cout << "IA planifica DISPARAR a (" << mejorX << "," << mejorY << ")" << std::endl;
        return AccionPlanificada(tanqueIA, "Atacar", mejorX, mejorY);
    }
    
    // Si no hay acción válida, retornar acción inválida
    return AccionPlanificada();
}

// Función modificada del menú del jugador para retornar la acción en lugar de ejecutarla
AccionPlanificada menuAccionesJugadorSimultaneo(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*> tanquesJugadorStack,
    NodoSistema* tablero,
    int filas,
    int columnas,
    int cellSize,
    sf::Texture& texturaTerreno1,
    sf::Texture& texturaTerreno2,
    sf::Texture& texturaTerreno3,
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA,
    sf::Texture& texturaExplosionTerreno,
    sf::Texture& texturaExplosionTanque
) {
    // ... (código de inicialización igual que antes)
    std::vector<Tanque*> tanquesJugador;
    std::stack<Tanque*> copia = tanquesJugadorStack;
    while (!copia.empty()) {
        tanquesJugador.push_back(copia.top());
        copia.pop();
    }
    std::reverse(tanquesJugador.begin(), tanquesJugador.end());

    if (tanquesJugador.empty()) {
        std::cout << "ERROR: No hay tanques para el jugador." << std::endl;
        return AccionPlanificada();
    }

    int indiceTanque = 0;
    int paso = 0;
    std::string accion = "Moverse";
    int coordX = 0, coordY = 0;
    bool coordenadaY = true;
    bool turnoCompletado = false;
    bool errorCoordenadas = false;

    const int infoY = 10;
    const int menuX = columnas * cellSize + 70;
    const int menuY = 100;
    const int offsetTableroX = 50;
    const int offsetTableroY = 90;

    sf::RectangleShape previewRect(sf::Vector2f(cellSize - 2, cellSize - 2));
    previewRect.setOutlineThickness(1);
    previewRect.setOutlineColor(sf::Color::Black);

    while (window.isOpen() && !turnoCompletado) {
        // ... (manejo de eventos igual que antes hasta la parte de ejecutar)
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) return AccionPlanificada();

                if (paso == 0) {
                    if (event.key.code == sf::Keyboard::Up)
                        indiceTanque = (indiceTanque - 1 + tanquesJugador.size()) % tanquesJugador.size();
                    if (event.key.code == sf::Keyboard::Down)
                        indiceTanque = (indiceTanque + 1) % tanquesJugador.size();
                    if (event.key.code == sf::Keyboard::Enter) {
                        paso = 1;
                        errorCoordenadas = false;
                    }
                }
                else if (paso == 1) {
                    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down)
                        accion = (accion == "Moverse") ? "Atacar" : "Moverse";
                    if (event.key.code == sf::Keyboard::Enter) {
                        paso = 2;
                        coordX = coordY = 0;
                        coordenadaY = false;
                        errorCoordenadas = false;
                    }
                }
                else if (paso == 2) {
                    // ... (manejo de entrada de coordenadas igual que antes)
                    int digit = -1;
                    if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                        digit = event.key.code - sf::Keyboard::Num0;
                    else if (event.key.code >= sf::Keyboard::Numpad0 && event.key.code <= sf::Keyboard::Numpad9)
                        digit = event.key.code - sf::Keyboard::Numpad0;

                    if (digit != -1 && digit >= 0 && digit < 5) { 
                        if (!coordenadaY && coordX == 0) coordX = digit;
                        else if (coordenadaY && coordY == 0) coordY = digit;
                    }

                    if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Right)
                        coordenadaY = !coordenadaY;

                    if (event.key.code == sf::Keyboard::Left)
                        coordenadaY = !coordenadaY;

                    if (event.key.code == sf::Keyboard::BackSpace) {
                        if (coordenadaY)
                            coordY = 0;
                        else
                            coordX = 0;
                    }

                    if (event.key.code == sf::Keyboard::Enter) {
                        if (coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
                            Tanque* tanqueSeleccionado = tanquesJugador[indiceTanque];
                            std::cout << "Jugador planifica " << accion << " en (" << coordX << "," << coordY << ")" << std::endl;
                            
                            // RETORNAR la acción en lugar de ejecutarla
                            return AccionPlanificada(tanqueSeleccionado, accion, coordX, coordY);
                        } else {
                            errorCoordenadas = true;
                        }
                    }
                }
            }
        }

        window.clear();

        // Recuadro superior de información
        const int infoPanelHeight = 50;
        sf::RectangleShape infoPanel(sf::Vector2f(window.getSize().x, infoPanelHeight));
        infoPanel.setPosition(0, 0);
        infoPanel.setFillColor(sf::Color(30, 30, 30, 220));
        infoPanel.setOutlineColor(sf::Color::White);
        infoPanel.setOutlineThickness(2);
        window.draw(infoPanel);

        // Texto de información de tanques
        for (size_t i = 0; i < tanquesJugador.size(); ++i) {
            std::string tipo;
            int dmg = tanquesJugador[i]->getDanio();
            if (dmg == 100) tipo = "Ligero";
            else if (dmg == 150) tipo = "Mediano";
            else if (dmg == 200) tipo = "Pesado";
            else tipo = "Desconocido";

            sf::Text info("Tanque " + std::to_string(i + 1) + " (" + tipo + ") Vida: " + std::to_string(tanquesJugador[i]->getVida()), font, 16);
            info.setPosition(20 + i * 250, 15);
            info.setFillColor(i == indiceTanque ? sf::Color::Yellow : sf::Color::White);
            window.draw(info);
        }

        // Dibujar el tablero
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA, 
            texturaExplosionTerreno, texturaExplosionTanque
        );

        // Mostrar preview de la acción si está en paso 2
        if (paso == 2 && coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
            previewRect.setPosition(coordX * cellSize + offsetTableroX + 1, coordY * cellSize + offsetTableroY + 1);
            previewRect.setFillColor((accion == "Moverse") ? sf::Color(0, 255, 0, 100) : sf::Color(255, 0, 0, 100));
            window.draw(previewRect);
        }

        // Dibujar menú lateral
        const int menuWidth = 230;
        const int menuHeight = 160;
        const int menuPadding = 10;

        sf::RectangleShape menuBackground(sf::Vector2f(menuWidth, menuHeight));
        menuBackground.setPosition(menuX - menuPadding, menuY - menuPadding);
        menuBackground.setFillColor(sf::Color(50, 50, 50, 200));
        menuBackground.setOutlineColor(sf::Color::White);
        menuBackground.setOutlineThickness(2);
        window.draw(menuBackground);

        sf::Text menu;
        menu.setFont(font);
        menu.setCharacterSize(20);
        menu.setFillColor(sf::Color::White);
        menu.setPosition(menuX, menuY);

        if (paso == 0)
            menu.setString("Selecciona un tanque\n(UP/DOWN)\nENTER para confirmar");
        else if (paso == 1)
            menu.setString("Accion: " + accion + "\n(UP/DOWN para cambiar)\nENTER para confirmar");
        else if (paso == 2) {
            std::string mensaje = "Ingresa coordenadas:\n";
            mensaje += "X: " + std::to_string(coordX) + "  Y: " + std::to_string(coordY);
            mensaje += "\nEditando: " + std::string(coordenadaY ? "Y" : "X");
            mensaje += "\nSPACE para cambiar entre X/Y\nENTER para ejecutar\nBACKSPACE para borrar";
            if (errorCoordenadas)
                mensaje += "\n[Coordenadas inválidas]";
            menu.setString(mensaje);
        }

        window.draw(menu);

        // Mostrar explosión si existe (copiado de tu código original)
        sf::Sprite spritePreviewExplosion;
        std::string textoExplosion;
        if (ultimaExplosion == TERRENO) {
            spritePreviewExplosion.setTexture(texturaExplosionTerreno);
            textoExplosion = "Disparo acerto al terreno";
        } else if (ultimaExplosion == TANQUE) {
            spritePreviewExplosion.setTexture(texturaExplosionTanque);
            textoExplosion = "Disparo acerto a un tanque";
        }
        if (ultimaExplosion != NINGUNA) {
            float explosionX = 475; 
            float explosionY = menuY + menuHeight + 50;
            spritePreviewExplosion.setScale(1.5f, 1.5f);
            spritePreviewExplosion.setPosition(explosionX, explosionY);

            // Texto arriba de la explosión
            sf::Text texto;
            texto.setFont(font);
            texto.setString(textoExplosion);
            texto.setCharacterSize(20);
            texto.setFillColor(sf::Color::White);
            texto.setPosition(explosionX + 100, explosionY + 80);
            window.draw(texto);

            window.draw(spritePreviewExplosion);
        }

        window.display();
    }
    
    return AccionPlanificada(); // Si sale del bucle sin completar
}

// Función para ejecutar las acciones simultáneamente
void ejecutarAccionesSimultaneas(
    AccionPlanificada accionJugador,
    AccionPlanificada accionIA,
    NodoSistema* tablero
) {
    std::cout << "\n=== EJECUTANDO ACCIONES SIMULTÁNEAS ===" << std::endl;
    
    bool exitoJugador = false;
    bool exitoIA = false;
    
    // Ejecutar acción del jugador
    if (accionJugador.esValida) {
        if (accionJugador.tipoAccion == "Moverse") {
            exitoJugador = ejecutarMovimiento(tablero, accionJugador.tanque, accionJugador.coordX, accionJugador.coordY);
            std::cout << "Jugador ejecuta movimiento: " << (exitoJugador ? "ÉXITO" : "FALLÓ") << std::endl;
        } else if (accionJugador.tipoAccion == "Atacar") {
            exitoJugador = ejecutarDisparo(tablero, accionJugador.tanque, accionJugador.coordX, accionJugador.coordY);
            std::cout << "Jugador ejecuta disparo: " << (exitoJugador ? "ÉXITO" : "FALLÓ") << std::endl;
        }
    }
    
    // Ejecutar acción de la IA
    if (accionIA.esValida) {
        if (accionIA.tipoAccion == "Moverse") {
            exitoIA = ejecutarMovimiento(tablero, accionIA.tanque, accionIA.coordX, accionIA.coordY);
            std::cout << "IA ejecuta movimiento: " << (exitoIA ? "ÉXITO" : "FALLÓ") << std::endl;
        } else if (accionIA.tipoAccion == "Atacar") {
            exitoIA = ejecutarDisparo(tablero, accionIA.tanque, accionIA.coordX, accionIA.coordY);
            std::cout << "IA ejecuta disparo: " << (exitoIA ? "ÉXITO" : "FALLÓ") << std::endl;
        }
    }
    
    std::cout << "=== FIN DE ACCIONES SIMULTÁNEAS ===\n" << std::endl;
}


bool menuAccionesJugador(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*> tanquesJugadorStack,
    NodoSistema* tablero,
    int filas,
    int columnas,
    int cellSize,
    sf::Texture& texturaTerreno1,
    sf::Texture& texturaTerreno2,
    sf::Texture& texturaTerreno3,
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA,
    sf::Texture& texturaExplosionTerreno,
    sf::Texture& texturaExplosionTanque
) {

    std::vector<Tanque*> tanquesJugador;
    {
        std::stack<Tanque*> copia = tanquesJugadorStack;
        while (!copia.empty()) {
            tanquesJugador.push_back(copia.top());
            copia.pop();
        }
    }
    std::reverse(tanquesJugador.begin(), tanquesJugador.end());
    if (tanquesJugador.empty()) return true;

    // Encuentra el primer tanque vivo
    int indiceTanque = 0;
    for (size_t i = 0; i < tanquesJugador.size(); ++i) {
        if (tanquesJugador[i]->getVida() > 0) {
            indiceTanque = i;
            break;
        }
    }

    int paso             = 0;
    std::string accion   = "Moverse";
    int coordX = 0, coordY = 0;
    bool coordYselect    = true;
    bool turnoCompletado = false;
    bool errorCoord      = false;
    int opcionConfirm    = 0;

    const int menuX = columnas * cellSize + 70;
    const int menuY = 100;
    const int offsetTableroX = 50;
    const int offsetTableroY = 90;
    const int menuWidth = 230;
    const int menuHeight = 160;
    const int menuPadding = 10;


    sf::RectangleShape previewRect({float(cellSize - 2), float(cellSize - 2)});
    previewRect.setOutlineThickness(1);
    previewRect.setOutlineColor(sf::Color::Black);


    sf::RectangleShape selectorRect({float(cellSize), float(cellSize)});
    selectorRect.setFillColor(sf::Color::Transparent);
    selectorRect.setOutlineThickness(3);
    selectorRect.setOutlineColor(sf::Color::Blue);

    while (window.isOpen() && !turnoCompletado) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return true;
            }
            
            if (event.type == sf::Event::KeyPressed) {

                if (event.key.code == sf::Keyboard::Escape) {
                    if (paso < 2) {
                        paso = 3;
                        opcionConfirm = 0;
                    } else {
                        paso = 1;
                        coordX = coordY = 0;
                        coordYselect = true;
                        errorCoord = false;
                    }
                    continue;
                }

                if (paso == 3) {
                    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) {
                        opcionConfirm = 1 - opcionConfirm;
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (opcionConfirm == 0) return false;
                        paso = 0;
                    }
                    continue;
                }

                if (paso == 0) {
                    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down) {
                        int direccion = (event.key.code == sf::Keyboard::Up) ? -1 : 1;
                        int intentos = 0;
                        do {
                            indiceTanque = (indiceTanque + direccion + tanquesJugador.size()) % tanquesJugador.size();
                            intentos++;
                        } while (tanquesJugador[indiceTanque]->getVida() <= 0 && intentos < tanquesJugador.size());
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        if (tanquesJugador[indiceTanque]->getVida() > 0) {
                            paso = 1;
                            errorCoord = false;
                        }
                    }
                }

                else if (paso == 1) {
                    if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down)
                        accion = (accion == "Moverse") ? "Atacar" : "Moverse";
                    else if (event.key.code == sf::Keyboard::Enter) {
                        paso = 2; coordX = coordY = 0; coordYselect = false; errorCoord = false;
                    }
                }
                else if (paso == 2) {
                    int digit = -1;
                    if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                        digit = event.key.code - sf::Keyboard::Num0;
                    else if (event.key.code >= sf::Keyboard::Numpad0 && event.key.code <= sf::Keyboard::Numpad9)
                        digit = event.key.code - sf::Keyboard::Numpad0;
                    if (digit >= 0 && digit < 5) {
                        if (!coordYselect && coordX == 0) coordX = digit;
                        else if (coordYselect && coordY == 0) coordY = digit;
                    }
                    else if (event.key.code == sf::Keyboard::Space ||
                             event.key.code == sf::Keyboard::Left ||
                             event.key.code == sf::Keyboard::Right) {
                        coordYselect = !coordYselect;
                    }
                    else if (event.key.code == sf::Keyboard::BackSpace) {
                        if (coordYselect) coordY = 0; else coordX = 0;
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
                            Tanque* t = tanquesJugador[indiceTanque];
                            if (accion == "Moverse") moverse(t, tablero, coordX, coordY);
                            else disparar(t, tablero, coordX, coordY);
                            turnoCompletado = true;
                        } else errorCoord = true;
                    }
                }
            }
        }

        window.clear();

        {
            sf::RectangleShape infoPanel({float(window.getSize().x), 50.f});
            infoPanel.setPosition(0, 0);
            infoPanel.setFillColor({30,30,30,220});
            infoPanel.setOutlineColor(sf::Color::White);
            infoPanel.setOutlineThickness(2);
            window.draw(infoPanel);


            for (size_t i = 0; i < tanquesJugador.size(); ++i) {
                int dmg = tanquesJugador[i]->getDanio();
                std::string tipo = (dmg == 100 ? "Ligero" : dmg == 150 ? "Mediano" : "Pesado");
                sf::Text info("Tanque " + std::to_string(i+1) +
                              " (" + tipo + ") Vida: " + std::to_string(tanquesJugador[i]->getVida()),
                              font, 16);
                info.setPosition(20 + i*250, 15);
                if (tanquesJugador[i]->getVida() <= 0)
                    info.setFillColor(sf::Color(128, 128, 128));  // Gris para tanque destruido
                else if (i == indiceTanque)
                    info.setFillColor(sf::Color::Yellow);         // Seleccionado
                else
                    info.setFillColor(sf::Color::White);

                window.draw(info);
            }
        }


        desplegarTablero(window, font, filas, columnas, cellSize, tablero,
                         texturaTerreno1, texturaTerreno2, texturaTerreno3,
                         texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
                         texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
                         texturaExplosionTerreno, texturaExplosionTanque);


        {
            Tanque* sel = tanquesJugador[indiceTanque];
            NodoSistema* nodo = tablero;
            while (nodo) {
                if (nodo->getTanque() == sel) {
                    int selX = nodo->getPosX();
                    int selY = nodo->getPosY();
                    selectorRect.setPosition(
                        float(selX*cellSize + offsetTableroX),
                        float(selY*cellSize + offsetTableroY)
                    );
                    window.draw(selectorRect);
                    break;
                }
                nodo = nodo->getSiguiente();
            }
        }

        // Preview movimiento/disparo
        if (paso == 2 && coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
            previewRect.setPosition(
                float(coordX*cellSize + offsetTableroX + 1),
                float(coordY*cellSize + offsetTableroY + 1)
            );
            previewRect.setFillColor(
                (accion=="Moverse") ? sf::Color(0,255,0,100) : sf::Color(255,0,0,100)
            );
            window.draw(previewRect);
        }

        // Menú de confirmación ESC
        if (paso == 3) {
            sf::RectangleShape bg({float(menuWidth), float(menuHeight)});
            bg.setPosition(menuX-menuPadding, menuY-menuPadding);
            bg.setFillColor({50,50,50,200});
            bg.setOutlineColor(sf::Color::White);
            bg.setOutlineThickness(2);
            window.draw(bg);

            sf::Text titulo("AVISO DE EMERGENCIA SOLDADO", font, 20);
            titulo.setFillColor(sf::Color::Red);
            titulo.setPosition(menuX, menuY);
            window.draw(titulo);

            sf::Text pregunta("¿DESEAS RENDIRTE ANTE EL ENEMIGO?", font, 18);
            pregunta.setFillColor(sf::Color::White);
            pregunta.setPosition(menuX, menuY + 30);
            window.draw(pregunta);

            sf::Text si("SI", font, 22), no("NO", font, 22);
            si.setPosition(menuX, menuY + 70);
            no.setPosition(menuX, menuY + 100);
            if (opcionConfirm == 0) si.setStyle(sf::Text::Underlined | sf::Text::Bold);
            else                    no.setStyle(sf::Text::Underlined | sf::Text::Bold);
            si.setFillColor(sf::Color::White);
            no.setFillColor(sf::Color::White);
            window.draw(si);
            window.draw(no);
        }
        else {
            // Menú normal
            sf::RectangleShape bg({float(menuWidth), float(menuHeight)});
            bg.setPosition(menuX-menuPadding, menuY-menuPadding);
            bg.setFillColor({50,50,50,200});
            bg.setOutlineColor(sf::Color::White);
            bg.setOutlineThickness(2);
            window.draw(bg);

            sf::Text menuTxt;
            menuTxt.setFont(font);
            menuTxt.setCharacterSize(20);
            menuTxt.setFillColor(sf::Color::White);
            menuTxt.setPosition(menuX, menuY);

            if (paso == 0) {
                menuTxt.setString(
                    "SELECCIONA TANQUE:\n"
                    "(ARRIBA/ABAJO)\n"
                    "ENTER=OK  ESC=Salir"
                );
            } else if (paso == 1) {
                menuTxt.setString(
                    std::string((accion=="Moverse") ? "> Moverse\n" : "  Moverse\n") +
                    std::string((accion=="Atacar")  ? "> Atacar\n"  : "  Atacar\n") +
                    "\nENTER=OK  ESC=Atras"
                );
            } else {
                menuTxt.setString(
                    "Ingresa coordenadas:\n" +
                    std::string("X: " + std::to_string(coordX) + "  Y: " + std::to_string(coordY)) +
                    "\nSPACE/LEFT/RIGHT alternar\nENTER=OK  ESC=Atras"
                );
            }
            window.draw(menuTxt);
        }

        // Sección explosión
        if (ultimaExplosion != NINGUNA) {
            sf::Sprite spr; std::string msg;
            if (ultimaExplosion == TERRENO) {
                spr.setTexture(texturaExplosionTerreno);
                msg = "Disparo acerto al terreno";
            } else {
                spr.setTexture(texturaExplosionTanque);
                msg = "Disparo acerto a un tanque";
            }
            float exX = 475, exY = menuY + menuHeight + 50;
            spr.setScale(1.5f, 1.5f);
            spr.setPosition(exX, exY);
            window.draw(spr);

            sf::Text te(msg, font, 20);
            te.setFillColor(sf::Color::White);
            te.setPosition(exX + 100, exY + 80);
            window.draw(te);
        }

        window.display();
    }

    return true;
}

// Función auxiliar para movimiento aleatorio cuando minimax falla
void movimientoAleatorioFallback(Tanque* tanqueIA, NodoSistema* tablero, int filas, int columnas) {
    // Encontrar nodo actual del tanque
    NodoSistema* origen = nullptr;
    NodoSistema* actual = tablero;
    while (actual) {
        if (actual->getTanque() == tanqueIA) {
            origen = actual;
            break;
        }
        actual = actual->getSiguiente();
    }
    
    if (!origen) return;

    // Decidir aleatoriamente: 0 = mover, 1 = disparar
    int accion = rand() % 2;

    if (accion == 0) {
        // Intentar moverse
        std::vector<NodoSistema*> opcionesMovimiento;
        
        // Direcciones: arriba, abajo, izquierda, derecha
        int direcciones[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
        
        for (int i = 0; i < 4; i++) {
            int newX = origen->getPosX() + direcciones[i][0];
            int newY = origen->getPosY() + direcciones[i][1];

            if (newX >= 0 && newX < columnas && newY >= 0 && newY < filas) {
                NodoSistema* destino = buscarNodo(tablero, newX, newY);
                if (destino && destino->getTanque() == nullptr) {
                    opcionesMovimiento.push_back(destino);
                }
            }
        }

        if (!opcionesMovimiento.empty()) {
            NodoSistema* destino = opcionesMovimiento[rand() % opcionesMovimiento.size()];
            moverseSimulado(tanqueIA, tablero, destino->getPosX(), destino->getPosY());
            std::cout << "Movimiento aleatorio a (" << destino->getPosX() << "," << destino->getPosY() << ")" << std::endl;
        }
    } else {
        // Intentar disparar
        std::vector<NodoSistema*> enemigosVivos;
        NodoSistema* temp = tablero;
        while (temp) {
            Tanque* t = temp->getTanque();
            if (t && !t->esIA() && t->getVida() > 0) {
                enemigosVivos.push_back(temp);
            }
            temp = temp->getSiguiente();
        }

        if (!enemigosVivos.empty()) {
            NodoSistema* objetivo = enemigosVivos[rand() % enemigosVivos.size()];
            dispararSimulado(tanqueIA, tablero, objetivo->getPosX(), objetivo->getPosY());
            std::cout << "Disparo aleatorio a (" << objetivo->getPosX() << "," << objetivo->getPosY() << ")" << std::endl;
        }
    }
}

void accionesIADificil(
    std::stack<Tanque*>& tanquesIA,
    NodoSistema* tablero,
    int filas,
    int columnas
) {
    // Obtener tanques vivos
    std::stack<Tanque*> copia = tanquesIA;
    std::vector<Tanque*> tanquesDisponibles;
    cout<<"aaaaaaaaaaa"<<endl;

    while (!copia.empty()) {
        if (copia.top()->getVida() > 0)
            tanquesDisponibles.push_back(copia.top());
        copia.pop();
    }

    if (tanquesDisponibles.empty()) return;

    // Elegir un tanque aleatorio
    Tanque* tanqueIA = tanquesDisponibles[rand() % tanquesDisponibles.size()];

    // Encontrar el tanque del jugador
    Tanque* tanqueJugador = nullptr;
    NodoSistema* temp = tablero;
    while (temp) {
        Tanque* t = temp->getTanque();
        if (t && !t->esIA() && t->getVida() > 0) {
            tanqueJugador = t;
            break;
        }
        temp = temp->getSiguiente();
    }

    cout<<"aaaaaaaax2"<<endl;
    if (!tanqueJugador) {
        std::cout << "No hay tanques enemigos vivos" << std::endl;
        return;
    }

    // CREAR ESTADO INICIAL CORRECTAMENTE
    // Nota: No clonamos aquí porque EstadoJuego se encarga de clonar
    EstadoJuego* estadoActual = new EstadoJuego(tablero, tanqueIA, tanqueJugador, true);
    

    cout<<"aaaaaaaax2.5"<<endl;
    // ENCONTRAR LA MEJOR JUGADA
    auto mejorJugada = encontrarMejorJugada(estadoActual, 6); // profundidad 4
    

    cout<<"aaaaaaaax2.6"<<endl;
    Accion mejorAccion = mejorJugada.first;
    int mejorX = mejorJugada.second.first;
    int mejorY = mejorJugada.second.second;


    cout<<"aaaaaaaax2.7"<<endl;
    // EJECUTAR LA ACCIÓN EN EL TABLERO REAL
    bool exito = false;
    

    cout<<"aaaaaaaax3"<<endl;
    if (mejorAccion == MOVER) {
        std::cout << "IA decide MOVERSE a (" << mejorX << "," << mejorY << ")" << std::endl;
        exito = ejecutarMovimiento(tablero, tanqueIA, mejorX, mejorY);
    } 
    else if (mejorAccion == DISPARAR) {
        std::cout << "IA decide DISPARAR a (" << mejorX << "," << mejorY << ")" << std::endl;
        exito = ejecutarDisparo(tablero, tanqueIA, mejorX, mejorY);
    }
    else {
        std::cout << "IA decide ESPERAR" << std::endl;
        exito = true; // Esperar siempre es válido
    }

    if (!exito) {
        std::cout << "Error: No se pudo ejecutar la acción de la IA" << std::endl;
        
        // FALLBACK: Si minimax falla, usar movimiento aleatorio
        std::cout << "Usando movimiento aleatorio como fallback..." << std::endl;
        movimientoAleatorioFallback(tanqueIA, tablero, filas, columnas);
    }

    // Liberar memoria
    //delete estadoActual;
}

bool mostrarMensajeFinal(sf::RenderWindow& window, sf::Font& font, const std::string& mensaje) {
    
    sf::Text texto;
    sf::Text info;

    if (mensaje == "Has ganado") {
        texto.setFillColor(sf::Color::Green);
    } else if (mensaje == "Has perdido") {
        texto.setFillColor(sf::Color::Red);
    } else {
        texto.setFillColor(sf::Color::White);
    }

    info.setFillColor(sf::Color::White);
    info.setString("Presiona ESC para volver al menu principal");

    auto centrarTexto = [](sf::Text& t) {
        sf::FloatRect bounds = t.getLocalBounds();
        t.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    };

    auto actualizarEstilo = [&](unsigned int ancho, unsigned int alto) {
        float proporcionAltura = static_cast<float>(alto);

        unsigned int tamTexto = static_cast<unsigned int>(proporcionAltura * 0.08f); // 8%
        unsigned int tamInfo = static_cast<unsigned int>(proporcionAltura * 0.035f); // 3.5%

        texto.setFont(font);
        texto.setString(mensaje);
        texto.setCharacterSize(tamTexto);
        texto.setStyle(sf::Text::Bold);

        info.setFont(font);
        info.setCharacterSize(tamInfo);

        centrarTexto(texto);
        centrarTexto(info);

        texto.setPosition(ancho / 2.f, alto / 2.f - proporcionAltura * 0.05f);
        info.setPosition(ancho / 2.f, alto / 2.f + proporcionAltura * 0.05f);
    };

    sf::Vector2u size = window.getSize();
    sf::FloatRect visibleArea(0, 0, size.x, size.y);
    window.setView(sf::View(visibleArea));
    actualizarEstilo(size.x, size.y);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                return true;
            } else if (event.type == sf::Event::Resized) {

                sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
                actualizarEstilo(event.size.width, event.size.height);
            }
        }

        window.clear(sf::Color::Black);
        window.draw(texto);
        window.draw(info);
        window.display();
    }

    return false;
}


// IA facil
bool menuAccionesIAFacil(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*> tanquesIAStack,
    std::stack<Tanque*> tanquesJugadorStack,
    NodoSistema* tablero,
    int filas,
    int columnas
) {
    // Convertir stacks a vectores para indexación
    std::vector<Tanque*> tanquesIA, tanquesJug;
    {
        auto copia = tanquesIAStack;
        while (!copia.empty()) {
            tanquesIA.push_back(copia.top());
            copia.pop();
        }
        std::reverse(tanquesIA.begin(), tanquesIA.end());
    }
    {
        auto copia = tanquesJugadorStack;
        while (!copia.empty()) {
            tanquesJug.push_back(copia.top());
            copia.pop();
        }
        std::reverse(tanquesJug.begin(), tanquesJug.end());
    }

    // Seleccionar primer tanque IA vivo
    Tanque* ia = nullptr;
    for (auto t : tanquesIA) {
        if (t->getVida() > 0) {
            ia = t;
            break;
        }
    }
    if (!ia) return true;

    // Crear lista de blancos vivos
    std::vector<Tanque*> vivos;
    for (auto t : tanquesJug) {
        if (t->getVida() > 0) {
            vivos.push_back(t);
        }
    }
    if (vivos.empty()) return true;

    // Escoger objetivo al azar
    size_t idx = static_cast<size_t>(std::rand()) % vivos.size();
    Tanque* objetivo = vivos[idx];

    // Encontrar posiciones (x,y) de IA y objetivo en el grafo
    int iaX = -1, iaY = -1, objX = -1, objY = -1;
    for (NodoSistema* n = tablero; n; n = n->getSiguiente()) {
        if (n->getTanque() == ia) {
            iaX = n->getPosX();
            iaY = n->getPosY();
        }
        if (n->getTanque() == objetivo) {
            objX = n->getPosX();
            objY = n->getPosY();
        }
        if (iaX >= 0 && objX >= 0) {
            break;
        }
    }
    if (objX < 0 || objY < 0) return true;

    // Lógica probabilística:
    //  - 60% disparo directo
    //  - 30% dispersión ±1
    //  - 10% no dispara
    int chance = std::rand() % 100;
    if (chance < 60) {
        // Disparo directo
        disparar(ia, tablero, objX, objY);
    }
    else if (chance < 90) {
        // Dispersión en [-1,0,1]
        int dx = (std::rand() % 3) - 1;
        int dy = (std::rand() % 3) - 1;
        int tx = std::clamp(objX + dx, 0, columnas - 1);
        int ty = std::clamp(objY + dy, 0, filas    - 1);
        disparar(ia, tablero, tx, ty);
    }
    else {
        // 10% pereza: no dispara este turno
    }

    return true;
}



bool menuAccionesIAMedia(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*> tanquesIAStack,
    std::stack<Tanque*> tanquesJugadorStack,
    NodoSistema* tablero,
    int filas,
    int columnas
) {
    std::vector<Tanque*> tanquesIA, tanquesJug;
    {
        auto copia = tanquesIAStack;
        while (!copia.empty()) {
            tanquesIA.push_back(copia.top());
            copia.pop();
        }
        std::reverse(tanquesIA.begin(), tanquesIA.end());
    }
    {
        auto copia = tanquesJugadorStack;
        while (!copia.empty()) {
            tanquesJug.push_back(copia.top());
            copia.pop();
        }
        std::reverse(tanquesJug.begin(), tanquesJug.end());
    }

    Tanque* ia = nullptr;
    for (auto t : tanquesIA) {
        if (t->getVida() > 0) {
            ia = t;
            break;
        }
    }
    if (!ia) return true;

    std::vector<Tanque*> vivos;
    for (auto t : tanquesJug) {
        if (t->getVida() > 0) {
            vivos.push_back(t);
        }
    }
    if (vivos.empty()) return true;

    // Buscar posiciones
    int iaX = -1, iaY = -1;
    for (NodoSistema* n = tablero; n; n = n->getSiguiente()) {
        if (n->getTanque() == ia) {
            iaX = n->getPosX();
            iaY = n->getPosY();
            break;
        }
    }

    // Buscar objetivo más cercano
    Tanque* objetivo = nullptr;
    int objX = -1, objY = -1;
    int minDistancia = INT_MAX;

    for (auto t : vivos) {
        for (NodoSistema* n = tablero; n; n = n->getSiguiente()) {
            if (n->getTanque() == t) {
                int x = n->getPosX();
                int y = n->getPosY();
                int distancia = std::abs(iaX - x) + std::abs(iaY - y);
                if (distancia < minDistancia) {
                    minDistancia = distancia;
                    objetivo = t;
                    objX = x;
                    objY = y;
                }
            }
        }
    }

    if (!objetivo || iaX < 0 || iaY < 0 || objX < 0 || objY < 0) return true;

    int chance = std::rand() % 100;

    if (minDistancia <= 2) {
        // En rango cercano: disparar
        if (chance < 75) {
            disparar(ia, tablero, objX, objY);
        } else {
            int dx = (std::rand() % 3) - 1;
            int dy = (std::rand() % 3) - 1;
            int tx = std::clamp(objX + dx, 0, columnas - 1);
            int ty = std::clamp(objY + dy, 0, filas - 1);
            disparar(ia, tablero, tx, ty);
        }
    } else {
        // A distancia: decidir entre moverse o disparar
        if (chance < 40) {
            disparar(ia, tablero, objX, objY);
        } else if (chance < 80) {
            int movX = (objX > iaX) ? 1 : (objX < iaX) ? -1 : 0;
            int movY = (objY > iaY) ? 1 : (objY < iaY) ? -1 : 0;
            int nuevoX = std::clamp(iaX + movX, 0, columnas - 1);
            int nuevoY = std::clamp(iaY + movY, 0, filas - 1);
            moverse(ia, tablero, nuevoX, nuevoY);
        } else {
            // 20% sin acción
        }
    }

    return true;
}

void bucleDeCombateSimultaneo(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*>& tanquesJugador,
    std::stack<Tanque*>& tanquesIA,
    NodoSistema* tablero,
    int filas,
    int columnas,
    int cellSize,
    sf::Texture& texturaTerreno1,
    sf::Texture& texturaTerreno2,
    sf::Texture& texturaTerreno3,
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA,
    sf::Texture& texturaExplosionTerreno,
    sf::Texture& texturaExplosionTanque
) {
    while (window.isOpen()) {
        // Verificar condiciones de victoria (igual que antes)
        bool jugadorSinTanques = true;
        {
            std::stack<Tanque*> tempJugador = tanquesJugador; 
            while (!tempJugador.empty()) {
                if (tempJugador.top()->getVida() > 0) {
                    jugadorSinTanques = false;
                    break;
                }
                tempJugador.pop();
            }
        }

        bool iaSinTanques = true;
        {
            std::stack<Tanque*> tempIA = tanquesIA;
            while (!tempIA.empty()) {
                if (tempIA.top()->getVida() > 0) {
                    iaSinTanques = false;
                    break;
                }
                tempIA.pop();
            }
        }

        if (jugadorSinTanques || iaSinTanques) {
            std::string mensajeFinal = jugadorSinTanques ? "Has perdido" : "Has ganado";
            bool volverAlMenu = mostrarMensajeFinal(window, font, mensajeFinal);
            if (volverAlMenu) {
                return; 
            }
        }

        // FASE 1: Obtener acción del jugador
        std::cout << "\n--- TURNO DEL JUGADOR ---" << std::endl;
        AccionPlanificada accionJugador = menuAccionesJugadorSimultaneo(
            window, font, tanquesJugador, tablero, filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
            texturaExplosionTerreno, texturaExplosionTanque
        );

        // FASE 2: Obtener acción de la IA
        std::cout << "\n--- TURNO DE LA IA ---" << std::endl;
        AccionPlanificada accionIA = obtenerAccionIA(tanquesIA, tablero, filas, columnas);

        // FASE 3: Ejecutar ambas acciones simultáneamente
        ejecutarAccionesSimultaneas(accionJugador, accionIA, tablero);

        // FASE 4: Mostrar resultado final
        window.clear();
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
            texturaExplosionTerreno, texturaExplosionTanque
        );
        window.display();
        
        // Pausa para que el jugador pueda ver el resultado
        sf::sleep(sf::milliseconds(1500));
    }
}

void bucleDeCombate(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*>& tanquesJugador,  // Pasar por referencia para que se mantenga el estado
    std::stack<Tanque*>& tanquesIA,       // Igual, referencia
    NodoSistema* tablero,
    int filas,
    int columnas,
    int cellSize,
    sf::Texture& texturaTerreno1,
    sf::Texture& texturaTerreno2,
    sf::Texture& texturaTerreno3,
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA,
    sf::Texture& texturaExplosionTerreno,
    sf::Texture& texturaExplosionTanque,
    int dificultad
) {
    while (window.isOpen()) {
        // Eventos
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                return;  // Salir del bucle de combate
            }
        }

        // Verificar si todos los tanques de un bando fueron destruidos
        bool jugadorSinTanques = true;
        {
            std::stack<Tanque*> tempJugador = tanquesJugador; 
            while (!tempJugador.empty()) {
                if (tempJugador.top()->getVida() > 0) {
                    jugadorSinTanques = false;
                    break;
                }
                tempJugador.pop();
            }
        }

        bool iaSinTanques = true;
        {
            std::stack<Tanque*> tempIA = tanquesIA;
            while (!tempIA.empty()) {
                if (tempIA.top()->getVida() > 0) {
                    iaSinTanques = false;
                    break;
                }
                tempIA.pop();
            }
        }

        if (jugadorSinTanques || iaSinTanques) {
            std::string mensajeFinal = jugadorSinTanques ? "Has perdido" : "Has ganado";
            bool volverAlMenu = mostrarMensajeFinal(window, font, mensajeFinal);
            if (volverAlMenu) {
                return;
            }
        }


        // FASE 1: Obtener acción del jugador
            std::cout << "\n--- TURNO DEL JUGADOR ---" << std::endl;
            AccionPlanificada accionJugador = menuAccionesJugadorSimultaneo(
                window, font, tanquesJugador, tablero, filas, columnas, cellSize,
                texturaTerreno1, texturaTerreno2, texturaTerreno3,
                texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
                texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
                texturaExplosionTerreno, texturaExplosionTanque
            );
        
        // Turno del jugador
        /*bool continuar = menuAccionesJugador(
            window, font, tanquesJugador, tablero,
            filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
            texturaExplosionTerreno, texturaExplosionTanque
        );*/
        if (!accionJugador.esValida) return;

        // Redibujar el tablero
        window.clear();
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
            texturaExplosionTerreno, texturaExplosionTanque
        );
        window.display();

        if (dificultad == 0) {
            // IA fácil
            menuAccionesIAFacil(
            window, font, tanquesIA, tanquesJugador,
            tablero, filas, columnas
            );

        } else if (dificultad == 1) {
            // IA media
            menuAccionesIAMedia(
                window, font, tanquesIA, tanquesJugador,
                tablero, filas, columnas
            );

        } else if (dificultad == 2) {
            // Turno de la IA - ejecutar acción difícil
            //accionesIADificil(tanquesIA, tablero, filas, columnas);

            

            // FASE 2: Obtener acción de la IA
            std::cout << "\n--- TURNO DE LA IA ---" << std::endl;
            AccionPlanificada accionIA = obtenerAccionIA(tanquesIA, tablero, filas, columnas);

            // FASE 3: Ejecutar ambas acciones simultáneamente
            ejecutarAccionesSimultaneas(accionJugador, accionIA, tablero);

            // Redibujar tablero luego de la acción de la IA
            // Pausa para que el jugador pueda ver el resultado
            //sf::sleep(sf::milliseconds(1500));
        }

        

        // Redibujar tras IA
        window.clear();
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA,
            texturaExplosionTerreno, texturaExplosionTanque
        );
        window.display();
    }
}

int main() {
    const int cellSize = 100;
    const int filas    = 5;
    const int columnas = 5;

    
    int anchoVentana = columnas * cellSize + 300;  
    int altoVentana  = filas * cellSize + 100;     

    sf::RenderWindow window(
        sf::VideoMode(anchoVentana, altoVentana),
        "Ferrum Bellum"
    );

    // Cargar fuente
    sf::Font font;
    if (!font.loadFromFile("Fuentes/BebasNeue-Regular.ttf")) {
        std::cout << "Error cargando la fuente." << std::endl;
        return -1;
    }

    mostrarPantallaInicio(window, font);

    while (window.isOpen()) {

        int opcion = mostrarMenuPrincipal(window, font);

        if (opcion == 0) {  
            sf::Music musicaFondo;
            if (!musicaFondo.openFromFile("Sonidos/musica_fondo.ogg")) {
                std::cout << "No se pudo cargar la música de fondo." << std::endl;
            } else {
                musicaFondo.setLoop(true); 
                musicaFondo.play();
            }
            int dificultad = mostrarMenuDificultad(window, font);
            if (dificultad == -1) {
                continue;  
            }
            // Pilas de tanques
            std::stack<Tanque*> tanquesJugador;
            std::stack<Tanque*> tanquesIA;

            // Cargar texturas
            sf::Texture texturaTerreno1, texturaTerreno2, texturaTerreno3;
            sf::Texture texturaTanque1Jugador, texturaTanque2Jugador, texturaTanque3Jugador;
            sf::Texture texturaTanque1IA, texturaTanque2IA, texturaTanque3IA, 
                        texturaExplosionTerreno, texturaExplosionTanque;
            if (!texturaTerreno1.loadFromFile("Imagenes/Terreno/planicie.png") ||
                !texturaTerreno2.loadFromFile("Imagenes/Terreno/bosque.png") ||
                !texturaTerreno3.loadFromFile("Imagenes/Terreno/montaniaNevada.png") ||
                !texturaTanque1Jugador.loadFromFile("Imagenes/Tanques/ligeroA-removebg-preview.png") ||
                !texturaTanque2Jugador.loadFromFile("Imagenes/Tanques/medianoA-removebg-preview.png") ||
                !texturaTanque3Jugador.loadFromFile("Imagenes/Tanques/pesadoA-removebg-preview.png") ||
                !texturaTanque1IA.loadFromFile("Imagenes/Tanques/ligeroR-removebg-preview.png") ||
                !texturaTanque2IA.loadFromFile("Imagenes/Tanques/medianoR-removebg-preview.png") ||
                !texturaTanque3IA.loadFromFile("Imagenes/Tanques/pesadoR-removebg-preview.png") ||
                !texturaExplosionTerreno.loadFromFile("Imagenes/Extras/explosion_terreno.png") ||
                !texturaExplosionTanque.loadFromFile("Imagenes/Extras/explosion_tanque.png")) {
                std::cout << "Error cargando texturas." << std::endl;
                return -1;
            }

            // Crear el tablero
            NodoSistema* tableroPosiciones = crearTablero();

            // Mostrar menú de selección de tanques
            bool seleccionOk = mostrarMenuSeleccionTanquesJugador(
                window, font, tanquesJugador, tableroPosiciones
            );
            if (!seleccionOk) {
                continue;
            }

            // Seleccionar tanques para IA y luego desplegar
            seleccionarTanquesIA(tanquesIA, tableroPosiciones);

            ultimaExplosion = NINGUNA;
            bucleDeCombate(
            window, font,
            tanquesJugador, tanquesIA, tableroPosiciones,
            filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanque1Jugador, texturaTanque2Jugador, texturaTanque3Jugador,
            texturaTanque1IA, texturaTanque2IA, texturaTanque3IA, texturaExplosionTerreno, texturaExplosionTanque,
            dificultad
            );

            /*
            bucleDeCombate(
            window, font,
            tanquesJugador, tanquesIA, tableroPosiciones,
            filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanque1Jugador, texturaTanque2Jugador, texturaTanque3Jugador,
            texturaTanque1IA, texturaTanque2IA, texturaTanque3IA, texturaExplosionTerreno, texturaExplosionTanque
            );*/

            // Liberar memoria de tanques
            while (!tanquesJugador.empty()) {
                delete tanquesJugador.top();
                tanquesJugador.pop();
            }
            while (!tanquesIA.empty()) {
                delete tanquesIA.top();
                tanquesIA.pop();
            }
        }
        else if (opcion == 1) {  
            window.close();
        }
    }

    return 0;
}