#include <SFML/Graphics.hpp>
#include <iostream>
#include "../Dominio/Tanques/TanquePesado.h"
#include "../Dominio/Nodos/NodoSistema.h"
using namespace std;

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

            NodoSistema* nuevoNodo = new NodoSistema(contador, i, j, tipoTerreno);
            listaPosiciones = agregarPosicion(listaPosiciones, nuevoNodo);
            contador++;
        }
    }

    return listaPosiciones;
}


void disparar(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    // Implementar la lógica de disparo aquí
    cout << "Tanque ID: " << tanque->getIdTanque() << " disparando!" << endl;

    NodoSistema* temp = tablero;
    TanquePesado* tanquePesado = dynamic_cast<TanquePesado*>(tanque);
    if (tanquePesado == nullptr) {
        cout << "Error: El tanque no es de tipo TanquePesado." << endl;
        return;
    }

    // Buscar la posición del tanque que dispara
    NodoSistema* tanqueDisparando = nullptr;
    NodoSistema* tempBusqueda = tablero;
    while (tempBusqueda != nullptr) {
        if (tempBusqueda->getTanque() == tanque) {
            tanqueDisparando = tempBusqueda;  // Nodo donde está el tanque que dispara
            break;
        }
        tempBusqueda = tempBusqueda->getSiguiente();
    }

    if (tanqueDisparando == nullptr) {
        cout << "Error: El tanque que dispara no se encuentra en el tablero." << endl;
        return;
    }

    // Mostrar la posición del tanque que dispara
    cout << "Tanque que dispara se encuentra en la posicion: (" 
         << tanqueDisparando->getPosX() << ", " << tanqueDisparando->getPosY() << ")" << endl;

    // Buscar el nodo donde se encuentra el tanque objetivo
    temp = tablero;
    srand(time(0));
    while (temp != nullptr) {
        if (temp->getPosX() == posX && temp->getPosY() == posY) {
            Tanque* tanqueEnPosicion = temp->getTanque();
            
            // Calcular la probabilidad del disparo (por ejemplo, afectado por terreno)
            int terrenoInicio = tanqueDisparando->getTipoTerreno();
            int terrenoFinal = temp->getTipoTerreno();
            double probabilidad = tanque->getProbabilidadDeImpacto(terrenoInicio, terrenoFinal);

            cout << "Probabilidad de acierto: " << probabilidad * 100 << "%" << endl;

            // Verificar si el disparo tiene éxito
            int numeroAleatorio = rand() % 100; // Generar un número aleatorio entre 0 y 99
            cout << "Numero aleatorio: " << numeroAleatorio << endl;
            if (numeroAleatorio < (probabilidad * 100)) { //Si el número generado es menor que la probabilidad de acierto, el disparo tiene éxito
                if (tanqueEnPosicion != nullptr) {
                    tanqueEnPosicion->actualizarVida(tanquePesado->getDaño());
                    cout << "Tanque ID: " << tanqueEnPosicion->getIdTanque() << " ha sido alcanzado!" << endl;
                    cout << "Vida restante: " << tanqueEnPosicion->getVida() << endl;
                } else {
                    cout << "¡Fallaste! No hay tanque en la posicion (" << posX << ", " << posY << ")." << endl;
                }
            } else {
                cout << "¡Fallaste! El disparo no tuvo exito." << endl;
            }
            break;
        }
    temp = temp->getSiguiente();
    }
}

/*int main() {
    // Crear el tablero y recorrerlo
    NodoSistema* tableroPosiciones = crearTablero();
    
    // Crear tanque pesado (con puntero)
    Tanque* tanque1 = new TanquePesado(1);
    Tanque* tanque2 = new TanquePesado(2);
    Tanque* tanque3 = new TanquePesado(3);
    Tanque* tanque4 = new TanquePesado(4);

    // Asignar tanques a ciertas posiciones del tablero
    NodoSistema* temp = tableroPosiciones;
    while (temp != nullptr) {
        int id = temp->getIdNodo();
    
        if (temp->getTanque() == nullptr) {
            if (id == 24) {
                temp->setTanque(tanque1);
            } else if (id == 0) {
                temp->setTanque(tanque2);
            } else if (id == 18) {
                temp->setTanque(tanque3);
            } else if (id == 13) {
                temp->setTanque(tanque4);
            }
        }
    
        temp = temp->getSiguiente();
    }
    
    // Disparar al tanque en la posición (0, 0)
    disparar(tanque1, tableroPosiciones, 2, 3); // Dispara al tanque en la posición (0, 0)
    return 0;
}*/

/*int main()
{
    const int cellSize = 100;
    const int gridSize = 3;
    const int windowSize = cellSize * gridSize;

    sf::RenderWindow window(sf::VideoMode(windowSize, windowSize), "Tablero 3x3");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        // Dibujar el tablero
        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
                sf::RectangleShape cell(sf::Vector2f(cellSize - 2, cellSize - 2));
                cell.setPosition(col * cellSize + 1, row * cellSize + 1);
                cell.setFillColor(sf::Color::Green);
                window.draw(cell);
            }
        }

        window.display();
    }

    return 0;
}*/

// ... (mismas funciones crearTablero, agregarPosicion, disparar)

sf::Color obtenerColorTerreno(int tipoTerreno) {
    switch (tipoTerreno) {
        case 1: return sf::Color(139, 69, 19); // Café oscuro: tierra
        case 2: return sf::Color(34, 139, 34); // Verde: pasto
        case 3: return sf::Color(169, 169, 169); // Gris: cemento
        default: return sf::Color::Black;
    }
}

int main() {
    const int cellSize = 100;
    const int gridRows = 5;
    const int gridCols = 5;
    const int windowWidth = cellSize * gridCols;
    const int windowHeight = cellSize * gridRows;

    // Crear tablero
    NodoSistema* tablero = crearTablero();

    // Crear tanques y asignarlos (opcional si quieres mostrar tanques también)
    Tanque* tanque1 = new TanquePesado(1);
    Tanque* tanque2 = new TanquePesado(2);
    Tanque* tanque3 = new TanquePesado(3);
    Tanque* tanque4 = new TanquePesado(4);

    NodoSistema* temp = tablero;
    while (temp != nullptr) {
        int id = temp->getIdNodo();
        if (temp->getTanque() == nullptr) {
            if (id == 24) temp->setTanque(tanque1);
            else if (id == 0) temp->setTanque(tanque2);
            else if (id == 18) temp->setTanque(tanque3);
            else if (id == 13) temp->setTanque(tanque4);
        }
        temp = temp->getSiguiente();
    }

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Tablero 5x5");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        // Recorrer el tablero y dibujar
        temp = tablero;
        while (temp != nullptr) {
            int x = temp->getPosX();
            int y = temp->getPosY();
            int terreno = temp->getTipoTerreno();

            sf::RectangleShape cell(sf::Vector2f(cellSize - 2, cellSize - 2));
            cell.setPosition(y * cellSize + 1, x * cellSize + 1); // y: columna, x: fila
            cell.setFillColor(obtenerColorTerreno(terreno));

            // Si hay tanque, cambia el color o dibuja algo adicional
            if (temp->getTanque() != nullptr) {
                cell.setOutlineThickness(3);
                cell.setOutlineColor(sf::Color::Red);
            }

            window.draw(cell);
            temp = temp->getSiguiente();
        }

        window.display();
    }

    return 0;
}

