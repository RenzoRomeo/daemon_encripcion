#include "daemonize.h"
#include "monitor.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
  // TODO: permitir iniciar y detener el daemon.
  // TODO: el daemon no debe poder estar instanciado más de una vez con el mismo
  // directorio de destino para cualquier momento dado.
  // TODO: permitir directorios

  if (argc != 3) {
    printf("Uso: daemon <directorio a supervisar> <directorio destino>\n");
    exit(EXIT_FAILURE);
  }

  char source_path[PATH_MAX];
  realpath(argv[1], source_path);
  // TODO: verificar que el directorio no esté siendo supervisado por otra
  // instancia del daemon.

  char destiny_path[PATH_MAX];
  realpath(argv[2], destiny_path);
  // TODO: si no existe el directorio, crearlo.
  // TODO: si el directorio no está vacío, salir con error.

  // TODO: pensar un nombre bueno
  const char *name = "File Enctryptor";
  daemonize(name);

  syslog(LOG_NOTICE, "%s started\n", name);

  monitor_directory(source_path, destiny_path);

  syslog(LOG_NOTICE, "%s terminated\n", name);
  closelog();

  return EXIT_SUCCESS;
}
