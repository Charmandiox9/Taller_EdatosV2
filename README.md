![SFML](https://img.shields.io/badge/SFML-2.6.1-green)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)

# Ferrum Bellum | Taller Estructura de Datos

## Compilación de Proyecto C++ con SFML 2.6.1 en MSYS2 (ucrt64)

Este proyecto utiliza la versión dinámica de SFML y está diseñado para compilarse en Windows mediante el entorno MSYS2 con el perfil `ucrt64`.

---

## 🛠 Tecnologías usadas

- Lenguaje: C++
- Librería gráfica: SFML
- IDE recomendado: Visual Studio Code
- Compilador: g++

## 📦 Estructura del proyecto
```
.
├── Dominio/    # Archivos .h y .cpp (Tanques y NodoSistema)
├── Fuentes/    # Fuente utilizada para los textos en pantalla.
├── Imagenes/   # Recursos gráficos para la interfaz.
├── Logica/     # Punto de entrada del programa.
├── SFML-2.6.1/ # Librería SFML.
└── README.md
```

## 🎮 Modo de juego
1. El jugador inicia en el menú principal con opciones de **Jugar** o **Salir**.
2. Jugador selecciona la dificultad de la IA.
3. Elige y posiciona sus tanques.
4. Se juegan rondas por turnos (moverse/disparar).
5. Los disparos se ejecutan de forma **simultánea**.

## 🧠 Inteligencia Artificial
- **Nivel 1 (Básico)**: Dispara en base a la posición del jugador. No considera vida ni movimientos. *(Enfoque probabilístico simple)*.
- **Nivel 2 (Intermedio)**: Considera posición y distancia del jugador, y decide si moverse o disparar. *(Probabilidad + lógica simple)*.
- **Nivel 3 (Avanzado)**: Tiene en cuenta todos los factores del juego: posición, distancias, vida propia y del rival, tipo de tanque, tipo de terreno, etc. *(Usa el algoritmo Minimax con poda alfa-beta)*.


## 🛠️ Requisitos

- Tener instalado [MSYS2](https://www.msys2.org/)
- Instalar SFML usando MSYS2:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-sfml
  

## Compilación

- Ejecuta el siguiente comando desde la raíz del proyecto:
  ```bash
  C:/msys64/ucrt64/bin/g++.exe -g Logica/main.cpp Dominio/Tanques/Tanque.cpp Dominio/Tanques/TanquePesado.cpp Dominio/Tanques/TanqueLigero.cpp Dominio/Tanques/TanqueMediano.cpp Dominio/Nodos/NodoSistema.cpp -I C:/msys64/ucrt64/include -o app.exe -L C:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lfreetype -lwinmm -lgdi32

## Ejecución

- Asegúrate de haber compilado correctamente (app.exe).
- Luego, ejecuta:
  ./app.exe

---
## Documentación de código

main.cpp
El archivo principal del juego que maneja la lógica con la que el usuario interactúa, componiéndose de la GUI, detección de eventos y la ejecución del juego en su totalidad.

Atributos
```bash
  explosiones: Contenedor de todas las explosiones activas en el tablero.
  ultimaExplosion: Almacena el tipo de la última explosión registrada.
```

Funciones
Main
```bash
  main(): Inicializa la ventana de juego, carga recursos (fuentes, música, texturas),
        muestra los menús de inicio y dificultad, permite la selección de tanques 
        del jugador y la IA, y ejecuta el bucle principal de combate.
        
        Al finalizar, libera la memoria de los tanques utilizados.
```

Funciones de generación de lista
```bash
  agregarPosicion(NodoSistema* head, NodoSistema* nuevo): Inserta un nodo nuevo al final de la lista doblemente enlazada. Recibe 2 Punteros de NodoSistema, uno es la cabeza de la lista, y el otro el nodo a agregar. Retorna el nodo head actualizado con el nuevo nodo.
  crearTablero(): Genera un tablero 5x5 con distintos tipos de terreno predefinidos por fila, filas 0 y 4 crea nodos con terreno "planicie", filas 1 y 3 crea nodos con terreno "bosque", fila 2 crea nodos con terreno "montaña". Retorna el nodo head que representa el tablero.
```
Funciones de acciones de tanques
```bash
  disparar(Tanque* tanque, NodoSistema* tablero, int posX, int posY): Permite que un tanque dispare a una coordenada del tablero, la probabilidad de acierto del disparo se verá afectada según un cáculo de probabilidad en base a los terrenos que este el origen y el destino. Recibe 1 puntero al tanque que esta ejecutando el disparo, recibe 1 puntero a la lista de nodos que representa el tablero, 2 enteros que indican las coordenadas X e Y. 
  moverse(Tanque* tanque, NodoSistema* tablero, int posX, int posY): Mueve un tanque a una nueva posición si está disponible, verifica si la nueva posición está disponible, desvincula el tanque de la posición anterior y lo vincula a la nueva. Recibe un puntero al tanque que se mueve, la lista de nodos que representa el tablero, 2 enteros que indican las coordenadas X e Y.
```

Funciones de desplegado de GUI
```bash
  mostrarMenuPrincipal(sf::RenderWindow& window, sf::Font& font): Muestra un menú principal interactivo en una ventana SFML con las ocpiones de "Jugar" y "Salir". Permite la navegación con las teclas de flecha ↑ ↓ y selección con la tecla Enter. También permite cerrar la ventana con el botón de cerrar. Recibe una referencia a la ventana SFML donde se msotrará el emnú (window) y una referencia a la fuente que usará para renderizar el texto (font). Retorna 0 (el usuario seleccionó "Jugar") o 1 (el usuario seleccionó "Salir" o cerro la ventana).
  mostrarMenuDificultad(sf::RenderWindow& window, sf::Font& font): Muestra un menú interactivo para seleccionar el nivel de dificultad del juego: Fácil, Media o Difícil. El usuario puede navegar con las teclas de flechas y confirmar con Enter. Si se cierra la ventana, se devuelve -1. Recibe una referencia a la ventana SFML donde se dibuja el menú (window) y una referencia a la fuente SFML usada para los textos (font). Retorna 0 (dificultad fácil seleccionada), 1 (dificultad media seleccionada), 2 (dificultad difícil seleccionada) o -1 (el usuario cerró la ventana sin hacer una selección).
  mostrarMenuSeleccionTanquesJugador(sf::RenderWindow& window, sf::Font& font, std::stack<Tanque*>& tanquesJugador, NodoSistema* tableroPosiciones): Muestra un menú interactivo que permite al jugador seleccionar tres tanques de entre tres tipos posibles: Ligero, Mediano y Pesado. Una vez seleccionado el tipo de tanque, el jugador debe ingresar las coordenadas de colocación (columna X y fila Y) mediante el teclado, permitiéndose sólo las filas 0 y 1. El usuario puede navegar por las opciones con las flechas ↑ y ↓, confirmar con Enter, cancelar la selección actual con Escape o eliminar el último tanque colocado con Backspace. Recibe una referencia a la ventana SFML donde se dibuja el menú (window), una referencia a la fuente SFML utilizada para los textos (font), una referencia a una pila donde se almacenan los tanques seleccionados por el jugador (tanquesJugador), un untero al primer nodo del sistema de nodos que representa el tablero (tableroPosiciones). Retorna true (si el jugador seleccionó y colocó tres tanques correctamente) o false (si cerró la ventana o canceló la selección).
  bucleDeCombate(sf::RenderWindow& window, sf::Font& font, std::stack<Tanque*> tanquesJugador, std::stack<Tanque*> tanquesIA, NodoSistema* tablero, int filas, int columnas, int cellSize, sf::Texture& texturaTerreno1, sf::Texture& texturaTerreno2, sf::Texture& texturaTerreno3, sf::Texture& texturaTanqueLigeroJugador, sf::Texture& texturaTanqueMedianoJugador, sf::Texture& texturaTanquePesadoJugador, sf::Texture& texturaTanqueLigeroIA, sf::Texture& texturaTanqueMedianoIA, sf::Texture& texturaTanquePesadoIA, sf::Texture& texturaExplosionTerreno, sf::Texture& texturaExplosionTanque): Ejecuta el bucle principal del combate entre el jugador y la IA. En cada iteración, verifica si alguno de los dos bandos perdió todos sus tanques (vida ≤ 0) para determinar el fin del juego y mostrar un mensaje final. Luego, permite que el jugador realice sus acciones a través de un menú interactivo, actualiza y redibuja el tablero con las texturas correspondientes, y finalmente realiza el turno de la IA (función de la IA pendiente de implementar). El bucle se repite mientras la ventana SFML esté abierta. Recibe una referencia a la ventana SFML donde se muestra el combate (window), una referencia a la fuente SFML para mostrar textos (font), una pila de punteros a los tanques del jugador (tanquesJugador), una pila de punteros a los tanques controlados por la IA (tanquesIA), un puntero al primer nodo que representa el tablero (tablero), un entero que representa el número de filas del tablero (filas), un entero que representa el número de columnas del tablero (columnas), un entero que representa el tamaño en pixeles de cada celda del tablero (cellSize), texturas para los distintos tipos de terreno (texturaTerreno1, texturaTerreno2, texturaTerreno3), texturas para los tanques del jugador según su tipo (texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador), texturas para los tanques de la IA según su tipo (texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA), texturas para animaciones de explosiones en terreno y tanques (texturaExplosionTerreno, texturaExplosionTanque).
  mostrarMensajeFinal(sf::RenderWindow& window, sf::Font& font, const std::string& mensaje): Muestra un mensaje final en el centro de la pantalla dentro de una ventana SFML, indicando si el jugador ha ganado, perdido u otro mensaje personalizado. También despliega una instrucción para regresar al menú principal al presionar la tecla ESC. La función detiene la ejecución hasta que se presione ESC o se cierre la ventana. Recibe una referencia a la ventana principal de SFML donde se renderiza el mensaje (window), una fuente que se usará para el texto principal e informativo (font) y un texto principal a mostrar(mensaje)
  menuAccionesJugador(sf::RenderWindow& window, sf::Font& font, std::stack<Tanque*> tanquesJugador, std::stack<Tanque*> tanquesIA, NodoSistema* tablero, int filas, int columnas, int cellSize, sf::Texture& texturaTerreno1, sf::Texture& texturaTerreno2, sf::Texture& texturaTerreno3, sf::Texture& texturaTanqueLigeroJugador, sf::Texture& texturaTanqueMedianoJugador, sf::Texture& texturaTanquePesadoJugador, sf::Texture& texturaTanqueLigeroIA, sf::Texture& texturaTanqueMedianoIA, sf::Texture& texturaTanquePesadoIA, sf::Texture& texturaExplosionTerreno, sf::Texture& texturaExplosionTanque): Esta función despliega un menú interactivo dentro de una ventana SFML, permitiendo al jugador seleccionar un tanque, decidir si desea moverse o atacar, e ingresar coordenadas para ejecutar la acción. Una vez realizada, la función finaliza y el turno del jugador se considera completado. Recibe una referencia a la ventana SFML donde se muestra el combate (window), una referencia a la fuente SFML para mostrar textos (font), una pila de punteros a los tanques del jugador (tanquesJugador), una pila de punteros a los tanques controlados por la IA (tanquesIA), un puntero al primer nodo que representa el tablero (tablero), un entero que representa el número de filas del tablero (filas), un entero que representa el número de columnas del tablero (columnas), un entero que representa el tamaño en pixeles de cada celda del tablero (cellSize), texturas para los distintos tipos de terreno (texturaTerreno1, texturaTerreno2, texturaTerreno3), texturas para los tanques del jugador según su tipo (texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador), texturas para los tanques de la IA según su tipo (texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA), texturas para animaciones de explosiones en terreno y tanques (texturaExplosionTerreno, texturaExplosionTanque).
  desplegarTablero(sf::RenderWindow& window, sf::Font& font, std::stack<Tanque*> tanquesJugador, std::stack<Tanque*> tanquesIA, NodoSistema* tablero, int filas, int columnas, int cellSize, sf::Texture& texturaTerreno1, sf::Texture& texturaTerreno2, sf::Texture& texturaTerreno3, sf::Texture& texturaTanqueLigeroJugador, sf::Texture& texturaTanqueMedianoJugador, sf::Texture& texturaTanquePesadoJugador, sf::Texture& texturaTanqueLigeroIA, sf::Texture& texturaTanqueMedianoIA, sf::Texture& texturaTanquePesadoIA, sf::Texture& texturaExplosionTerreno, sf::Texture& texturaExplosionTanque): Dibuja el tablero en la ventana SFML incluyendo coordenadas X e Y del tablero, terreno según tipo, tanques de la IA y jugador con la textura correspondiente. Recibe una referencia a la ventana SFML donde se muestra el combate (window), una referencia a la fuente SFML para mostrar textos (font), una pila de punteros a los tanques del jugador (tanquesJugador), una pila de punteros a los tanques controlados por la IA (tanquesIA), un puntero al primer nodo que representa el tablero (tablero), un entero que representa el número de filas del tablero (filas), un entero que representa el número de columnas del tablero (columnas), un entero que representa el tamaño en pixeles de cada celda del tablero (cellSize), texturas para los distintos tipos de terreno (texturaTerreno1, texturaTerreno2, texturaTerreno3), texturas para los tanques del jugador según su tipo (texturaTanqueLigeroJugador, texturaTanqueMedianoJugador, texturaTanquePesadoJugador), texturas para los tanques de la IA según su tipo (texturaTanqueLigeroIA, texturaTanqueMedianoIA, texturaTanquePesadoIA), texturas para animaciones de explosiones en terreno y tanques (texturaExplosionTerreno, texturaExplosionTanque).
```
---

## Autores

- [Daniel Durán](https://github.com/Charmandiox9) | 21.493.398-5 | ITI
- [Martín Castillo](https://github.com/Marton1123) | 21.484.992-5 | ITI

> [!IMPORTANT]
> Este proyecto está en desarrollo, por lo que puede tener bugs...
