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
daemon start <directorio a supervisar> <directorio destino>
```

### Detener instancia

```bash
daemon stop <directorio destino>
```


## Características
- Permite ejecutar más de una instancia en simultáneo, con la única restricción de que dos instancias no pueden utilizar el mismo directorio de destino al mismo tiempo.
- Utiliza modulo de kernel de Linux "inotify".
- Bajo uso de recursos, debido a la llamada bloqueante a "read" y uso de polling.
