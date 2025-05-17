# Ferrum Bellum

## Taller Estructura de Datos

# Compilación de Proyecto C++ con SFML 2.6.1 en MSYS2 (ucrt64)

Este proyecto utiliza la versión dinámica de SFML y está diseñado para compilarse en Windows mediante el entorno MSYS2 con el perfil `ucrt64`.

---

## 🛠️ Requisitos

- [MSYS2](https://www.msys2.org/)
- SFML instalado vía MSYS2:
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-sfml
  ```

## Compilación

- Debera ejecutar el siguiente comando en la terminal de la raiz del proyecto:
  C:/msys64/ucrt64/bin/g++.exe -g Logica/main.cpp Dominio/Tanques/Tanque.cpp Dominio/Tanques/TanquePesado.cpp Dominio/Tanques/TanqueLigero.cpp Dominio/Tanques/TanqueMediano.cpp Dominio/Nodos/NodoSistema.cpp -I C:/msys64/ucrt64/include -o app.exe -L C:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lfreetype -lwinmm -lgdi32

## Ejecución

Para ejecutar el programa:

- Asegúrate de que app.exe se haya generado correctamente.
- Ejecuta el comando
  ./app.exe

## Autores

- Daniel Durán
- Martín Castillo

> [!IMPORTANT]
> Este proyecto está en desarrollo, por lo que puede tener bugs...
