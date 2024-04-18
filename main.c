#include "daemonize.h"
#include "monitor.h"

#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // TODO: el daemon no debe poder estar instanciado más de una vez con el mismo
  // directorio de destino para cualquier momento dado.
  //
  // 1. el destination path identifica unívocamente a cada instancia del daemon.
  // 2. almacenar en un archivo (pid_file) un mapa {destination_path: pid}
  // 3. al ejecutar daemon stop <destination_path>, parsear archivo y hacer
  // kill(pid) de la instancia que corresponda.
  //
  // TODO: permitir directorios
  //
  // TODO: usos:
  //  daemon start <source_dir> <destination_dir> (argc == 4)
  //  daemon stop <destination_dir> (argc == 4)

  if (argc != 3) {
    printf("Uso: daemon <directorio a supervisar> <directorio destino>\n");
    exit(EXIT_FAILURE);
  }

  char source_path[PATH_MAX];
  realpath(argv[1], source_path);
  // TODO: verificar que el directorio no esté siendo supervisado por otra
  // instancia del daemon.

  char destination_path[PATH_MAX];
  realpath(argv[2], destination_path);

  int result = rmdir(destination_path);
  if (result != 0 && errno == ENOTEMPTY) {
    syslog(LOG_ERR, "Destination directory is not empty: %s\n",
           destination_path);
    printf("Destination directory is not empty: %s\n", destination_path);
    exit(EXIT_FAILURE);
  }

  struct stat st = {0};
  if (stat(destination_path, &st) == -1) {
    mkdir(destination_path, 0700);
    syslog(LOG_NOTICE, "Created destination directory: %s\n", destination_path);
  }

  // TODO: pensar un nombre bueno
  const char *name = "File Enctryptor";
  daemonize(name);

  syslog(LOG_NOTICE, "%s started\n", name);

  monitor_directory(source_path, destination_path);

  syslog(LOG_NOTICE, "%s terminated\n", name);
  closelog();

  return EXIT_SUCCESS;
}
