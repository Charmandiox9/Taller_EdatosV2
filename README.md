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
.
├── Dominio/    # Archivos .h y .cpp (Tanques y NodoSistema)
├── Fuentes/    # Fuente utilizada para los textos en pantalla.
├── Imagenes/   # Recursos gráficos para la interfaz.
├── Logica/     # Punto de entrada del programa.
├── SFML-2.6.1/ # Librería SFML.
└── README.md

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
  ```

## Compilación

- Ejecuta el siguiente comando desde la raíz del proyecto:
  C:/msys64/ucrt64/bin/g++.exe -g Logica/main.cpp Dominio/Tanques/Tanque.cpp Dominio/Tanques/TanquePesado.cpp Dominio/Tanques/TanqueLigero.cpp Dominio/Tanques/TanqueMediano.cpp Dominio/Nodos/NodoSistema.cpp -I C:/msys64/ucrt64/include -o app.exe -L C:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lfreetype -lwinmm -lgdi32

## Ejecución

- Asegúrate de haber compilado correctamente (app.exe).
- Luego, ejecuta:
  ./app.exe

## Autores

- [Daniel Durán](https://github.com/Charmandiox9)
- [Martín Castillo](https://github.com/Marton1123)

> [!IMPORTANT]
> Este proyecto está en desarrollo, por lo que puede tener bugs...
