#include "daemonize.h"
#include "monitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
  // TODO: permitir iniciar y detener el daemon.

  if (argc != 3) {
    printf("Uso: daemon <directorio a supervisar> <directorio destino>\n");
    exit(EXIT_FAILURE);
  }

  const char *source_path = argv[1];
  // TODO: permitir rutas relativas a la ubicación actual del ejecutable.
  // TODO: verificar que exista el directorio.
  // TODO: verificar que el directorio no esté siendo supervisado por otra
  // instancia del daemon.

  const char *destiny_path = argv[2];
  // TODO: si no existe el directorio, crearlo.
  // TODO: si el directorio no está vacío, salir con error.

  daemonize("File Encryptor");

  syslog(LOG_NOTICE, "First daemon started.");

  monitor_directory(source_path, destiny_path);

  syslog(LOG_NOTICE, "First daemon terminated.");
  closelog();

  return EXIT_SUCCESS;
}
