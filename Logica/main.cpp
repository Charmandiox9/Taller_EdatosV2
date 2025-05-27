#include <SFML/Graphics.hpp>
#include <iostream>
#include <stack>
#include <vector>
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


// Función que mueve un tanque a una posición específica
void moverse(Tanque* tanque, NodoSistema* tablero, int posX, int posY) {
    cout << "Tanque ID: " << tanque->getIdTanque() << " intentando moverse a (" << posX << ", " << posY << ")" << endl;

    NodoSistema* destino = nullptr;
    NodoSistema* origen = nullptr;

    NodoSistema* temp = tablero;
    while (temp != nullptr) {
        if (temp->getTanque() == tanque) {
            origen = temp;  // Nodo donde está el tanque actualmente
        }
        if (temp->getPosX() == posX && temp->getPosY() == posY) {
            destino = temp; // Nodo destino
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

    if (origen) {
        origen->setTanque(nullptr);  // Desocupa la posición anterior
    }

    destino->setTanque(tanque);  // Mueve el tanque a la nueva posición
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

    const int cellSize = 40;
    const int filas = 5, columnas = 5;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // Liberar memoria al cerrar
                for (auto t : seleccionados) delete t;
                delete tanqueParaColocar;
                window.close();
                return false;
            }

            if (estado == SELECCION_TANQUE && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    if (seleccionados.size() == 3) {
                        // Confirmar selección: pasar punteros a la pila
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
                                // NO hacemos delete aquí para evitar crash
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
                    // Cancelar selección: liberar memoria
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
                        if (!escribiendoY) inputX += c;
                        else inputY += c;
                    } else if (c == ' ' && !inputX.empty() && !escribiendoY) {
                        escribiendoY = true;
                    }
                } else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Backspace) {
                        if (escribiendoY && !inputX.empty()) {
                            inputX.pop_back();
                        } else if (escribiendoY && inputX.empty()) {
                            escribiendoY = false;
                            if (!inputY.empty()) inputY.pop_back();
                        } else if (!escribiendoY && !inputY.empty()) {
                            inputY.pop_back();
                        }
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

        // --- Dibujo ---
        window.clear(sf::Color::Black);


        sf::Text titulo("Selecciona tus tanques", font, 32);
        titulo.setFillColor(sf::Color(110, 180, 100));
        titulo.setPosition(100, 50);
        window.draw(titulo);

        sf::Text volver("Volver (ESC)", font, 18);
        volver.setFillColor(sf::Color(180, 180, 180));
        volver.setPosition(10, 10);
        window.draw(volver);

        if (estado == SELECCION_TANQUE) {
            for (int i = 0; i < (int)opciones.size(); ++i) {
                sf::Text txt(opciones[i], font, 26);
                txt.setPosition(100, 120 + i * 50);

                if (i == seleccion) {
                    sf::FloatRect bounds = txt.getLocalBounds();
                    sf::RectangleShape highlight(sf::Vector2f(bounds.width + 20, bounds.height + 18));
                    highlight.setFillColor(sf::Color(110, 180, 100));  // Verde militar
                    highlight.setPosition(txt.getPosition().x - 10, txt.getPosition().y - 8);
                    window.draw(highlight);

                    txt.setFillColor(sf::Color::Black);
                } else {
                    txt.setFillColor(sf::Color::White);
                }

                window.draw(txt);
            }
        } else {
            sf::Text info("Ingresa coordenadas: X (columna) Y (fila) - Solo filas 0 y 1", font, 20);
            info.setFillColor(sf::Color::Green);
            info.setPosition(100, 140);
            window.draw(info);

            sf::Text entrada("X: " + inputX + " Y: " + inputY, font, 20);
            entrada.setFillColor(sf::Color::White);
            entrada.setPosition(100, 170);
            window.draw(entrada);

            if (tanqueParaColocar) {
                sf::Text stats(
                    "Vida: " + std::to_string(tanqueParaColocar->getVida()) +
                    "  Danio: " + std::to_string(tanqueParaColocar->getDanio()) +
                    "  Mov: " + std::to_string(tanqueParaColocar->getMovimientoBase()),
                    font, 18);
                stats.setFillColor(sf::Color::Cyan);
                stats.setPosition(100, 210);
                window.draw(stats);
            }
        }

        // Cantidad de tanques seleccionados
        sf::Text count("Tanques elegidos: " + std::to_string(seleccionados.size()), font, 20);
        count.setFillColor(sf::Color::White);
        count.setPosition(100, 300);
        window.draw(count);

        float offsetY = 330;  
        float ancho = columnas * cellSize;
        float offsetX = (window.getSize().x - ancho) / 2.f;
        NodoSistema* cur = tableroPosiciones;
        while (cur) {
            sf::RectangleShape sq({(float)cellSize, (float)cellSize});
            sq.setPosition(offsetX + cur->getPosX() * cellSize,
                           offsetY + cur->getPosY() * cellSize);
            sq.setFillColor(cur->getTanque() ? sf::Color::Red : sf::Color::White);
            sq.setOutlineColor(sf::Color::Black);
            sq.setOutlineThickness(1);
            window.draw(sq);
            cur = cur->getSiguiente();
        }

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

    // Parámetro de filas (ajústalo según tu tablero real)
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
    sf::Texture& texturaTanquePesadoIA
) {
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

        float posX = offsetTableroX - 25;  // a la izquierda del tablero
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
        if (actual->getTanque() != nullptr) {
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
}


int mostrarMenuDificultad(sf::RenderWindow& window, sf::Font& font) {
    std::vector<std::string> opciones = {"Fácil", "Media", "Difícil"};
    int seleccion = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return -1;  // Se cerró la ventana
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    return seleccion;  // 0 = Fácil, 1 = Media, 2 = Difícil
                }
            }
        }

        window.clear(sf::Color::Black);

        // Título
        sf::Text titulo("Selecciona la Dificultad", font, 44);
        titulo.setFillColor(sf::Color(110, 180, 100));
        titulo.setStyle(sf::Text::Bold);
        sf::FloatRect boundsTitulo = titulo.getLocalBounds();
        titulo.setOrigin(boundsTitulo.width / 2, boundsTitulo.height / 2);
        titulo.setPosition(window.getSize().x / 2, 100);
        window.draw(titulo);

        for (int i = 0; i < (int)opciones.size(); ++i) {
            sf::Text texto(opciones[i], font, 32);
            texto.setStyle(sf::Text::Bold);
            sf::FloatRect bounds = texto.getLocalBounds();
            texto.setOrigin(bounds.width / 2, bounds.height / 2);
            texto.setPosition(window.getSize().x / 2, 220 + i * 70);

            if (i == seleccion) {
                sf::RectangleShape highlight(sf::Vector2f(bounds.width + 20, bounds.height + 20));
                highlight.setFillColor(sf::Color(110, 180, 100));
                highlight.setOrigin(highlight.getSize().x / 2, highlight.getSize().y / 2);
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

    return -1;
}

int mostrarMenuPrincipal(sf::RenderWindow& window, sf::Font& font) {
    std::vector<std::string> opciones = {"Jugar", "Salir"};
    int seleccion = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return 1;  // Se cerró la ventana
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    seleccion = (seleccion - 1 + opciones.size()) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Down) {
                    seleccion = (seleccion + 1) % opciones.size();
                } else if (event.key.code == sf::Keyboard::Enter) {
                    return seleccion;  // 0 = Jugar, 1 = Salir
                }
            }
        }

        window.clear(sf::Color::Black);

        // Título centrado
        sf::Text titulo("FERRUM BELLUM", font, 54);
        titulo.setFillColor(sf::Color(110, 180, 100));  // Verde militar
        titulo.setStyle(sf::Text::Bold);
        sf::FloatRect boundsTitulo = titulo.getLocalBounds();
        titulo.setOrigin(boundsTitulo.width / 2, boundsTitulo.height / 2);
        titulo.setPosition(window.getSize().x / 2, 100);
        window.draw(titulo);

        // Opciones del menú
        for (int i = 0; i < (int)opciones.size(); ++i) {
            sf::Text texto(opciones[i], font, 32);
            texto.setStyle(sf::Text::Bold);
            sf::FloatRect bounds = texto.getLocalBounds();
            texto.setOrigin(bounds.width / 2, bounds.height / 2);
            texto.setPosition(window.getSize().x / 2, 220 + i * 70);

            if (i == seleccion) {
                // Rectángulo de fondo con color verde militar
                sf::RectangleShape highlight(sf::Vector2f(bounds.width + 20, bounds.height + 20));
                highlight.setFillColor(sf::Color(110, 180, 100));
                highlight.setOrigin(highlight.getSize().x / 2, highlight.getSize().y / 2);
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

void menuAccionesJugador(
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
    sf::Texture& texturaTanquePesadoIA
) {
    std::vector<Tanque*> tanquesJugador;
    std::stack<Tanque*> copia = tanquesJugadorStack;
    while (!copia.empty()) {
        tanquesJugador.push_back(copia.top());
        copia.pop();
    }
    std::reverse(tanquesJugador.begin(), tanquesJugador.end());

    if (tanquesJugador.empty()) {
        std::cout << "ERROR: No hay tanques para el jugador." << std::endl;
        return;
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
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) return;

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
                    int digit = -1;
                    if (event.key.code >= sf::Keyboard::Num0 && event.key.code <= sf::Keyboard::Num9)
                        digit = event.key.code - sf::Keyboard::Num0;
                    else if (event.key.code >= sf::Keyboard::Numpad0 && event.key.code <= sf::Keyboard::Numpad9)
                        digit = event.key.code - sf::Keyboard::Numpad0;

                    if (digit != -1) {
                        if (coordenadaY && coordY < 100)
                            coordY = coordY * 10 + digit;
                        else if (!coordenadaY && coordX < 100)
                            coordX = coordX * 10 + digit;
                    }

                    if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Right)
                        coordenadaY = !coordenadaY;

                    if (event.key.code == sf::Keyboard::Left)
                        coordenadaY = true;

                    if (event.key.code == sf::Keyboard::BackSpace) {
                        if (coordenadaY)
                            coordY /= 10;
                        else
                            coordX /= 10;
                    }

                    if (event.key.code == sf::Keyboard::Enter) {
                        if (coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
                            Tanque* tanqueSeleccionado = tanquesJugador[indiceTanque];
                            if (accion == "Moverse"){
                                moverse(tanqueSeleccionado, tablero, coordX, coordY);
                                std::cout << "Movimiento completado. X=" << coordX << ", Y=" << coordY << std::endl;
                            }else {
                                disparar(tanqueSeleccionado, tablero, coordX, coordY);
                            }  
                            turnoCompletado = true;
                            // DEBUG EXTRA
                            std::cout << "¿Se sigue ejecutando después de moverse?" << std::endl;
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
        //cout<<"Llega aca"<<endl;
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA
        );
        //cout<<"Llega aca x2"<<endl;
        if (paso == 2 && coordX >= 0 && coordX < columnas && coordY >= 0 && coordY < filas) {
            previewRect.setPosition(coordX * cellSize + offsetTableroX + 1, coordY * cellSize + offsetTableroY + 1);
            previewRect.setFillColor((accion == "Moverse") ? sf::Color(0, 255, 0, 100) : sf::Color(255, 0, 0, 100));
            window.draw(previewRect);
        }

        const int menuWidth = 230;
        const int menuHeight = 150;
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
        //cout<<"Llega aca x3"<<endl;
        if (paso == 0)
            menu.setString("Selecciona un tanque\n(UP/DOWN)\nENTER para confirmar");
        else if (paso == 1)
            menu.setString("Acción: " + accion + "\n(UP/DOWN para cambiar)\nENTER para confirmar");
        else if (paso == 2) {
            std::string mensaje = "Ingresa coordenadas:\n";
            mensaje += "X: " + std::to_string(coordX) + "  Y: " + std::to_string(coordY);
            mensaje += "\nSPACE para cambiar entre X/Y\nENTER para ejecutar\nBACKSPACE para borrar";
            if (errorCoordenadas)
                mensaje += "\n[Coordenadas inválidas]";
            menu.setString(mensaje);
        }

        window.draw(menu);
        window.display();
    }
}

void actualizarYMostrarTablero(
    sf::RenderWindow& window,
    sf::Font& font,
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
    sf::Texture& texturaTanquePesadoIA
) {
    window.clear();
    desplegarTablero(
        window, font, filas, columnas, cellSize, tablero,
        texturaTerreno1, texturaTerreno2, texturaTerreno3,
        texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
        texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA
    );
    window.display();
    sf::sleep(sf::seconds(0.5));
}


void bucleDeCombate(
    sf::RenderWindow& window,
    sf::Font& font,
    std::stack<Tanque*> tanquesJugador,
    std::stack<Tanque*> tanquesIA,
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
    sf::Texture& texturaTanquePesadoIA
) {
    while (window.isOpen()) {
        // Verificar si todos los tanques de un bando fueron destruidos
        bool jugadorSinTanques = true;
        std::stack<Tanque*> tempJugador = tanquesJugador;
        while (!tempJugador.empty()) {
            if (tempJugador.top()->getVida() > 0) {
                jugadorSinTanques = false;
                break;
            }
            tempJugador.pop();
        }

        bool iaSinTanques = true;
        std::stack<Tanque*> tempIA = tanquesIA;
        while (!tempIA.empty()) {
            if (tempIA.top()->getVida() > 0) {
                iaSinTanques = false;
                break;
            }
            tempIA.pop();
        }

        if (jugadorSinTanques || iaSinTanques) {
            std::string mensajeFinal = jugadorSinTanques ? "¡Has perdido!" : "¡Has ganado!";
            std::cout << mensajeFinal << std::endl;
            // Aquí podrías dibujar un mensaje en pantalla en lugar de solo consola
            return;
        }

        // Turno del jugador
        menuAccionesJugador(
            window, font, tanquesJugador, tablero, filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA
        );

        // Redibujar el tablero después del turno del jugador
        window.clear();
        desplegarTablero(
            window, font, filas, columnas, cellSize, tablero,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador,
            texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA
        );
        window.display();

        // Turno de la IA (si tienes una función de IA)
        // menuAccionesIA(...); // Aquí llamas a la IA si la tienes implementada


        // Si usas lógica de IA, también redibuja después:
        // window.clear();
        // desplegarTablero(...);
        // window.display();
    }
}



int main() {
    const int cellSize = 100;
    const int filas    = 5;
    const int columnas = 5;

    // Ajustar tamanio ventana para info arriba y menú a la derecha
    int anchoVentana = columnas * cellSize + 300;  // 300 px para menú derecho y margen
    int altoVentana  = filas * cellSize + 100;     // 100 px para info arriba y margen

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

    while (window.isOpen()) {
        int opcion = mostrarMenuPrincipal(window, font);

        if (opcion == 0) {  // Jugar
            int dificultad = mostrarMenuDificultad(window, font);
            if (dificultad == -1) {
                continue;  // Se cerró la ventana
            }
            // Pilas de tanques
            std::stack<Tanque*> tanquesJugador;
            std::stack<Tanque*> tanquesIA;

            // Cargar texturas
            sf::Texture texturaTerreno1, texturaTerreno2, texturaTerreno3;
            sf::Texture texturaTanque1Jugador, texturaTanque2Jugador, texturaTanque3Jugador;
            sf::Texture texturaTanque1IA, texturaTanque2IA, texturaTanque3IA;
            if (!texturaTerreno1.loadFromFile("Imagenes/Terreno/planicie.png") ||
                !texturaTerreno2.loadFromFile("Imagenes/Terreno/bosque.png") ||
                !texturaTerreno3.loadFromFile("Imagenes/Terreno/montaniaNevada.png") ||
                !texturaTanque1Jugador.loadFromFile("Imagenes/Tanques/ligeroA-removebg-preview.png") ||
                !texturaTanque2Jugador.loadFromFile("Imagenes/Tanques/medianoA-removebg-preview.png") ||
                !texturaTanque3Jugador.loadFromFile("Imagenes/Tanques/pesadoA-removebg-preview.png") ||
                !texturaTanque1IA.loadFromFile("Imagenes/Tanques/ligeroR-removebg-preview.png") ||
                !texturaTanque2IA.loadFromFile("Imagenes/Tanques/medianoR-removebg-preview.png") ||
                !texturaTanque3IA.loadFromFile("Imagenes/Tanques/pesadoR-removebg-preview.png")) {
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

            // Seleccionar IA y luego desplegar
            seleccionarTanquesIA(tanquesIA, tableroPosiciones);

            bucleDeCombate(
            window, font,
            tanquesJugador, tanquesIA, tableroPosiciones,
            filas, columnas, cellSize,
            texturaTerreno1, texturaTerreno2, texturaTerreno3,
            texturaTanque1Jugador, texturaTanque2Jugador, texturaTanque3Jugador,
            texturaTanque1IA, texturaTanque2IA, texturaTanque3IA
            );

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
        else if (opcion == 1) {  // Salir
            window.close();
        }
    }

    return 0;
}