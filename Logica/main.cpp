#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time()

#include "../Dominio/Tanques/Tanque.h"
#include "../Dominio/Tanques/TanqueLigero.h"
#include "../Dominio/Tanques/TanqueMediano.h"
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

// Función que dispara un tanque a una posición específica
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


void mostrarMenuSeleccionTanquesJugador(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*>& tanquesJugador,
    NodoSistema* tableroPosiciones
) {
    std::vector<std::string> opciones = {"Tanque Ligero", "Tanque Mediano", "Tanque Pesado"};
    int seleccion = 0;
    int idTanque = 1;
    std::vector<Tanque*> seleccionados;

    enum Estado { SELECCION_TANQUE, INGRESO_COORDENADAS } estado = SELECCION_TANQUE;

    Tanque* tanqueParaColocar = nullptr;
    std::string inputX = "";
    std::string inputY = "";
    bool escribiendoY = false;

    const int cellSize = 40;
    const int filas = 5;
    const int columnas = 5;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                for (auto t : seleccionados) delete t;
                if (tanqueParaColocar) delete tanqueParaColocar;
                window.close();
                return;
            }

            if (estado == SELECCION_TANQUE) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        seleccion = (seleccion + 1) % opciones.size();
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (seleccionados.size() == 3) {
                            for (int i = seleccionados.size() - 1; i >= 0; --i) {
                                tanquesJugador.push(seleccionados[i]);
                            }
                            return;
                        } else {
                            switch (seleccion) {
                                case 0: tanqueParaColocar = new TanqueLigero(idTanque++); break;
                                case 1: tanqueParaColocar = new TanqueMediano(idTanque++); break;
                                case 2: tanqueParaColocar = new TanquePesado(idTanque++); break;
                            }
                            inputX = "";
                            inputY = "";
                            escribiendoY = false;
                            estado = INGRESO_COORDENADAS;
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Backspace) {
                        if (!seleccionados.empty()) {
                            delete seleccionados.back();
                            seleccionados.pop_back();
                        }
                    }
                }
            }
            else if (estado == INGRESO_COORDENADAS) {
                if (event.type == sf::Event::TextEntered) {
                    char entered = static_cast<char>(event.text.unicode);
                    if (isdigit(entered)) {
                        if (!escribiendoY)
                            inputY += entered;  // CORREGIDO: primero Y
                        else
                            inputX += entered;  // luego X
                    }
                    else if (entered == ' ') {
                        if (!inputY.empty() && !escribiendoY) {
                            escribiendoY = true;
                        }
                    }
                }
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        if (escribiendoY && !inputX.empty()) {
                            inputX.pop_back();
                        } else if (escribiendoY && inputX.empty()) {
                            escribiendoY = false;
                            if (!inputY.empty())
                                inputY.pop_back();
                        } else if (!escribiendoY && !inputY.empty()) {
                            inputY.pop_back();
                        }
                    }
                    else if (event.key.code == sf::Keyboard::Enter) {
                        if (!inputX.empty() && !inputY.empty()) {
                            int y = std::stoi(inputY);  // CORREGIDO: y primero
                            int x = std::stoi(inputX);  // luego x

                            NodoSistema* temp = tableroPosiciones;
                            NodoSistema* nodoDestino = nullptr;
                            while (temp != nullptr) {
                                if (temp->getPosY() == x && temp->getPosX() == y) {
                                    nodoDestino = temp;
                                    break;
                                }
                                temp = temp->getSiguiente();
                            }

                            if (nodoDestino == nullptr || nodoDestino->getTanque() != nullptr || y >= 2) {
                                inputX = "";
                                inputY = "";
                                escribiendoY = false;
                            } else {
                                nodoDestino->setTanque(tanqueParaColocar);
                                seleccionados.push_back(tanqueParaColocar);
                                tanqueParaColocar = nullptr;
                                estado = SELECCION_TANQUE;
                            }
                        }
                    }
                }
            }
        }

        window.clear(sf::Color::Black);

        sf::Text titulo("Selecciona tus tanques", font, 30);
        titulo.setFillColor(sf::Color::White);
        titulo.setPosition(100, 30);
        window.draw(titulo);

        if (estado == SELECCION_TANQUE) {
            for (int i = 0; i < opciones.size(); ++i) {
                sf::Text texto(opciones[i], font, 24);
                texto.setPosition(100, 100 + i * 40);
                texto.setFillColor(i == seleccion ? sf::Color::Yellow : sf::Color::White);
                window.draw(texto);
            }
        }
        else if (estado == INGRESO_COORDENADAS) {
            sf::Text info("Ingresa coordenadas Y X (ej: 1 0). Solo filas 0 y 1", font, 20);
            info.setFillColor(sf::Color::Green);
            info.setPosition(100, 100);
            window.draw(info);

            sf::Text inputTexto(("Y: " + inputY + "  X: " + inputX), font, 20);
            inputTexto.setFillColor(sf::Color::White);
            inputTexto.setPosition(100, 140);
            window.draw(inputTexto);

            if (tanqueParaColocar != nullptr) {
                sf::Text stats(
                    "Vida: " + std::to_string(tanqueParaColocar->getVida()) +
                    " | Daño: " + std::to_string(tanqueParaColocar->getDaño()) +
                    " | Movimiento: " + std::to_string(tanqueParaColocar->getMovimientoBase()),
                    font, 18
                );
                stats.setFillColor(sf::Color::Cyan);
                stats.setPosition(100, 180);
                window.draw(stats);
            }
        }

        sf::Text info2("Tanques seleccionados: " + std::to_string(seleccionados.size()), font, 20);
        info2.setFillColor(sf::Color::White);
        info2.setPosition(100, 250);
        window.draw(info2);

        float offsetY = 250 + 30;
        float anchoTablero = columnas * cellSize;
        float offsetX = (window.getSize().x - anchoTablero) / 2.0f;

        NodoSistema* temp = tableroPosiciones;
        while (temp != nullptr) {
            sf::RectangleShape cuadro(sf::Vector2f(cellSize, cellSize));
            cuadro.setPosition(offsetX + temp->getPosY() * cellSize, offsetY + temp->getPosX() * cellSize);
            cuadro.setFillColor(temp->getTanque() ? sf::Color::Red : sf::Color::White);
            cuadro.setOutlineColor(sf::Color::Black);
            cuadro.setOutlineThickness(1);
            window.draw(cuadro);
            temp = temp->getSiguiente();
        }

        window.display();
    }
}


// Función que selecciona tanques para la IA
void seleccionarTanquesIA(
    std::stack<Tanque*>& tanquesIA,
    NodoSistema* tableroPosiciones
) {
    std::vector<std::string> tiposTanque = {"Ligero", "Mediano", "Pesado"};
    int idTanque = 100;

    // Parámetro de filas (ajústalo según tu tablero real)
    const int columnas = 5;
    const int filas = 5;

    std::vector<NodoSistema*> nodosDisponibles;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    NodoSistema* temp = tableroPosiciones;
    while (temp != nullptr) {
        int x = temp->getPosX();
        if (temp->getTanque() == nullptr && (x == columnas - 1 || x == columnas - 2)) {
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
    sf::Texture& texturaTanqueLigero,
    sf::Texture& texturaTanqueMediano,
    sf::Texture& texturaTanquePesado
) {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Dibujar coordenadas X
        for (int col = 0; col < columnas; col++) {
            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(col));
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::White);

            sf::FloatRect bounds = text.getLocalBounds();
            float posX = col * cellSize + 50 + (cellSize - bounds.width) / 2.0f - bounds.left;
            text.setPosition(posX, 10);
            window.draw(text);
        }

        // Dibujar coordenadas Y
        for (int row = 0; row < filas; row++) {
            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(row));
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::White);

            sf::FloatRect bounds = text.getLocalBounds();
            float posY = row * cellSize + 50 + (cellSize - bounds.height) / 2.0f - bounds.top;
            text.setPosition(10, posY);
            window.draw(text);
        }

        NodoSistema* actual = tableroPosiciones;
        while (actual != nullptr) {
            sf::Sprite sprite;
            switch (actual->getTipoTerreno()) {
                case 1: sprite.setTexture(texturaTerreno1); break;
                case 2: sprite.setTexture(texturaTerreno2); break;
                case 3: sprite.setTexture(texturaTerreno3); break;
                default: sprite.setColor(sf::Color::Red); break;
            }

            float x = actual->getPosY() * cellSize + 50;
            float y = actual->getPosX() * cellSize + 50;
            sprite.setPosition(x, y);
            sprite.setScale(
                (float)cellSize / sprite.getTexture()->getSize().x,
                (float)cellSize / sprite.getTexture()->getSize().y
            );
            window.draw(sprite);

            if (actual->getTanque() != nullptr) {
                sf::Sprite spriteTanque;
                if(actual ->getTanque()->getMovimientoBase() == 6){
                    spriteTanque.setTexture(texturaTanqueLigero);
                } else if(actual ->getTanque()->getMovimientoBase() == 4){
                    spriteTanque.setTexture(texturaTanqueMediano);
                } else if(actual ->getTanque()->getMovimientoBase() == 2){
                    spriteTanque.setTexture(texturaTanquePesado);
                }

                
                
                float scaleFactor = 0.5f;

                float tanqueScaleX = scaleFactor * ((float)cellSize / spriteTanque.getTexture()->getSize().x);
                float tanqueScaleY = scaleFactor * ((float)cellSize / spriteTanque.getTexture()->getSize().y);

                spriteTanque.setScale(tanqueScaleX, tanqueScaleY);

                float offsetX = (cellSize - spriteTanque.getTexture()->getSize().x * tanqueScaleX) / 2.0f;
                float offsetY = (cellSize - spriteTanque.getTexture()->getSize().y * tanqueScaleY) / 2.0f;
                spriteTanque.setPosition(x + offsetX, y + offsetY);
                window.draw(spriteTanque);
            }

            actual = actual->getSiguiente();
        }

        window.display();
    }
}


int main() {

    stack <Tanque*> tanquesJugador;
    stack <Tanque*> tanquesIA;

    const int cellSize = 100;
    const int filas = 5;
    const int columnas = 5;

    sf::RenderWindow window(sf::VideoMode(columnas * cellSize + 50, filas * cellSize + 50), "Ferrum Bellum");

    // Cargar fuente
    sf::Font font;
    if (!font.loadFromFile("Fuentes/BebasNeue-Regular.ttf")) {
        std::cout << "Error cargando la fuente." << std::endl;
        return -1;
    }

    // Cargar texturas
    sf::Texture texturaTerreno1, texturaTerreno2, texturaTerreno3, 
    texturaTanque1Jugadores, texturaTanque2Jugadores, texturaTanque3Jugadores,
    texturaTanque4IA, texturaTanque5IA, texturaTanque6IA;
    if (!texturaTerreno1.loadFromFile("Imagenes/Terreno/planicie.png") ||
        !texturaTerreno2.loadFromFile("Imagenes/Terreno/bosque.png") ||
        !texturaTerreno3.loadFromFile("Imagenes/Terreno/montaniaNevada.png") ||
        !texturaTanque1Jugadores.loadFromFile("Imagenes/Tanques/ligeroA-removebg-preview.png") ||
        !texturaTanque2Jugadores.loadFromFile("Imagenes/Tanques/medianoA-removebg-preview.png") ||
        !texturaTanque3Jugadores.loadFromFile("Imagenes/Tanques/pesadoA-removebg-preview.png") ||
        !texturaTanque4IA.loadFromFile("Imagenes/Tanques/ligeroR-removebg-preview.png") ||
        !texturaTanque5IA.loadFromFile("Imagenes/Tanques/medianoR-removebg-preview.png") ||
        !texturaTanque6IA.loadFromFile("Imagenes/Tanques/pesadoR-removebg-preview.png")) {
        std::cout << "Error cargando una o más texturas." << std::endl;
        return -1;
    }

    // Crear tablero
    NodoSistema* tableroPosiciones = crearTablero(); // Asegúrate de que esta función esté implementada

    // Mostrar menú inicial
    mostrarMenuSeleccionTanquesJugador(window, font, tanquesJugador, tableroPosiciones);
    seleccionarTanquesIA(tanquesIA, tableroPosiciones);

    // Asignar tanques
    Tanque* tanque1 = new TanquePesado(1);
    Tanque* tanque2 = new TanquePesado(2);
    Tanque* tanque3 = new TanquePesado(3);
    Tanque* tanque4 = new TanquePesado(4);


    // Desplegar tablero
    desplegarTablero(window, font, filas, columnas, cellSize, tableroPosiciones,
                     texturaTerreno1, texturaTerreno2, texturaTerreno3,
                     texturaTanque1Jugadores, texturaTanque2Jugadores, texturaTanque3Jugadores);

    return 0;
}