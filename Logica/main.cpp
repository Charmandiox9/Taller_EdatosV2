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
                        if (!escribiendoY) inputY += c;
                        else inputX += c;
                    } else if (c == ' ' && !inputY.empty() && !escribiendoY) {
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
                                if (temp->getPosY() == x && temp->getPosX() == y) {
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

        // Título (igual al menú principal en color verde militar)
        sf::Text titulo("Selecciona tus tanques", font, 32);
        titulo.setFillColor(sf::Color(110, 180, 100));
        titulo.setPosition(100, 50);
        window.draw(titulo);

        // Texto volver (ESC) en gris claro
        sf::Text volver("Volver (ESC)", font, 18);
        volver.setFillColor(sf::Color(180, 180, 180));  // Gris claro
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
            sf::Text info("Ingresa Y X (ej: 1 0)", font, 20);
            info.setFillColor(sf::Color::Green);
            info.setPosition(100, 140);
            window.draw(info);

            sf::Text entrada("Y: " + inputY + " X: " + inputX, font, 20);
            entrada.setFillColor(sf::Color::White);
            entrada.setPosition(100, 170);
            window.draw(entrada);

            if (tanqueParaColocar) {
                sf::Text stats(
                    "Vida: " + std::to_string(tanqueParaColocar->getVida()) +
                    "  Daño: " + std::to_string(tanqueParaColocar->getDaño()) +
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
            sq.setPosition(offsetX + cur->getPosY() * cellSize,
                           offsetY + cur->getPosX() * cellSize);
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
    // texturas jugador
    sf::Texture& texturaTanqueLigeroJugador,
    sf::Texture& texturaTanqueMedianoJugador,
    sf::Texture& texturaTanquePesadoJugador,
    // texturas IA
    sf::Texture& texturaTanqueLigeroIA,
    sf::Texture& texturaTanqueMedianoIA,
    sf::Texture& texturaTanquePesadoIA
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
            sf::Text text(std::to_string(col), font, 18);
            text.setFillColor(sf::Color::White);
            sf::FloatRect bounds = text.getLocalBounds();
            float posX = col * cellSize + 50 + (cellSize - bounds.width) / 2.0f - bounds.left;
            text.setPosition(posX, 10);
            window.draw(text);
        }

        // Dibujar coordenadas Y
        for (int row = 0; row < filas; row++) {
            sf::Text text(std::to_string(row), font, 18);
            text.setFillColor(sf::Color::White);
            sf::FloatRect bounds = text.getLocalBounds();
            float posY = row * cellSize + 50 + (cellSize - bounds.height) / 2.0f - bounds.top;
            text.setPosition(10, posY);
            window.draw(text);
        }

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
            float x = actual->getPosY() * cellSize + 50;
            float y = actual->getPosX() * cellSize + 50;
            sprite.setPosition(x, y);
            sprite.setScale(
                (float)cellSize / sprite.getTexture()->getSize().x,
                (float)cellSize / sprite.getTexture()->getSize().y
            );
            window.draw(sprite);

            // Tanque
            if (actual->getTanque() != nullptr) {
                sf::Sprite spriteTanque;
                Tanque* t = actual->getTanque();
                bool esIA = t->getIdTanque() >= 100;
                int mov = t->getMovimientoBase();

                // Elegir textura
                if (mov == 6) {
                    spriteTanque.setTexture(
                        esIA ? texturaTanqueLigeroIA : texturaTanqueLigeroJugador
                    );
                } else if (mov == 4) {
                    spriteTanque.setTexture(
                        esIA ? texturaTanqueMedianoIA : texturaTanqueMedianoJugador
                    );
                } else if (mov == 2) {
                    spriteTanque.setTexture(
                        esIA ? texturaTanquePesadoIA : texturaTanquePesadoJugador
                    );
                }

                // Escala 50%
                float scaleX = 0.5f * (float)cellSize / spriteTanque.getTexture()->getSize().x;
                float scaleY = 0.5f * (float)cellSize / spriteTanque.getTexture()->getSize().y;
                spriteTanque.setScale(scaleX, scaleY);

                if (!esIA) {
                    // Jugador: gira 180° y centra
                    auto ts = spriteTanque.getTexture()->getSize();
                    spriteTanque.setOrigin(ts.x / 2.f, ts.y / 2.f);
                    spriteTanque.setPosition(x + cellSize / 2.f, y + cellSize / 2.f);
                    spriteTanque.setRotation(180.f);
                } else {
                    // IA: sin giro, centrado sencillo
                    float offsetX = (cellSize - spriteTanque.getTexture()->getSize().x * scaleX) / 2.f;
                    float offsetY = (cellSize - spriteTanque.getTexture()->getSize().y * scaleY) / 2.f;
                    spriteTanque.setPosition(x + offsetX, y + offsetY);
                }

                window.draw(spriteTanque);
            }

            actual = actual->getSiguiente();
        }

        window.display();
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



int main() {
    const int cellSize = 100;
    const int filas    = 5;
    const int columnas = 5;

    sf::RenderWindow window(
        sf::VideoMode(columnas * cellSize + 50, filas * cellSize + 50),
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
            if(dificultad == -1) {
                continue;  // Se cerró la ventana
            }
            // Pilas de tanques
            stack<Tanque*> tanquesJugador; 
            stack<Tanque*> tanquesIA;

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

            desplegarTablero(
                window, font,
                filas, columnas, cellSize,
                tableroPosiciones,
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