# Daemon para encriptar (pensar nombre)

Este daemon tiene como objetivo supervisar un directorio del sistema de archivos, y mantener una copia de la carpeta con todos los contenidos encriptados.
Para ello hace uso del API de inotify, para escuchar los eventos de creación, modificación y eliminación de archivos en el directorio a supervisar y creando copias encripatadas en el directorio de destino.

## Instalación

1. Clonar repositorio.
2. Ejecutar script de instalación `build.sh`.

## Uso

### Mensaje de ayuda

```bash
daemon
```

### Iniciar instancia

```bash
daemon start <directorio a supervisar> <directorio destino> <clave de encriptado>
```

### Detener instancia

```bash
daemon stop <directorio destino>
```

### Desencriptar directorio encriptado

```bash
decrypt <directorio destino> <clave de encriptado>
```


## Características
- Permite ejecutar más de una instancia en simultáneo, con la única restricción de que dos instancias no pueden utilizar el mismo directorio de destino al mismo tiempo.
- Archivos encriptados en base a una clave alfanumérica provista por el usuario.
- Herramienta para desencriptar directorio encriptado, utilizando la misma clave usada para encriptar.
- Utiliza modulo de kernel de Linux "inotify".
- Bajo uso de recursos, debido a la llamada bloqueante a "read" y uso de polling.
