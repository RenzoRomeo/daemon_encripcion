# Daemon para encriptar (pensar nombre)

Este daemon tiene como objetivo supervisar un directorio del sistema de archivos, y mantener una copia de la carpeta con todos los contenidos encriptados.
Para ello hace uso del API de inotify, para escuchar los eventos de creación, modificación y eliminación de archivos en el directorio a supervisar y creando copias encripatadas en el directorio de destino.

## Uso

```bash
daemon <directorio a supervisar> <directorio destino>
```