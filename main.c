#include "daemonize.h"
#include "monitor.h"

#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

const char *pid_path = "./.daemon_pid";

static pid_t destination_to_pid(const char *destination) {
  FILE *fp = fopen(pid_path, "rt");

  if (fp == NULL) {
    return -1;
  }

  char *dest_line = NULL;
  size_t bytes = 0;

  int found = 0;
  pid_t pid = -1;
  while (!found && getline(&dest_line, &bytes, fp) > 0) {
    if (strcmp(dest_line, destination) == 0) {
      char *pid_line = NULL;
      getline(&pid_line, 0, fp);
      pid = atoi(pid_line);
      found = 1;
      free(pid_line);
    }
    free(dest_line);
    bytes = 0;
  }

  fclose(fp);

  return pid;
}

static void store_destination(const char *destination) {
  FILE *fp = fopen(pid_path, "w+");
  if (fp == NULL) {
    // TODO: manejar error
  }

  fprintf(fp, "%s\n%d\n", destination, getpid());
  fclose(fp);
}

static int start(const char *source_dir, const char *destination_dir) {
  char source_path[PATH_MAX];
  realpath(source_dir, source_path);
  // TODO: verificar que el directorio no esté siendo supervisado por otra
  // instancia del daemon.

  char destination_path[PATH_MAX];
  realpath(destination_dir, destination_path);

  pid_t pid = destination_to_pid(destination_path);

  if (pid > 0) {
    syslog(LOG_ERR,
           "Destination directory is already in use by another instance of the "
           "daemon: %s\n",
           destination_path);
    printf("Destination directory is already in use by another instance of the "
           "daemon: %s\n",
           destination_path);
    exit(EXIT_FAILURE);
  }

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

  store_destination(destination_path);

  // TODO: pensar un nombre bueno
  const char *name = "File Enctryptor";
  daemonize(name);

  syslog(LOG_NOTICE,
         "Daemon instance started with source '%s' and destination: '%s'\n",
         source_path, destination_path);

  monitor_directory(source_path, destination_path);

  syslog(LOG_NOTICE, "%s terminated\n", name);
  closelog();

  return EXIT_SUCCESS;
}

static int stop(const char *destination_dir) {
  char destination_path[PATH_MAX];
  realpath(destination_dir, destination_path);

  pid_t pid = destination_to_pid(destination_path);
  if (pid < 0) {
    syslog(LOG_ERR,
           "Destination directory is not being used by any instance of the "
           "daemon: %s\n",
           destination_path);
    printf("Destination directory is not being used by any instance of the "
           "daemon: %s\n",
           destination_path);
    exit(EXIT_FAILURE);
  }

  // TODO: borrar entrada de esta instancia del daemon en el pid_file.

  kill(pid, SIGKILL);

  syslog(LOG_NOTICE, "Daemon instance stopped: %s\n", destination_path);

  return 0;
}

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

  if (argc < 2) {
    printf("Usage: daemon <start | stop> <...>\n");
    exit(EXIT_FAILURE);
  }

  if (strcmp(argv[1], "start") == 0) {
    if (argc != 4) {
      printf("Usage: daemon start <directorio a supervisar> <directorio "
             "destino>\n");
      exit(EXIT_FAILURE);
    }
    const char *source_dir = argv[2];
    const char *destination_dir = argv[3];
    return start(source_dir, destination_dir);
  } else if (strcmp(argv[1], "stop") == 0) {
    if (argc != 3) {
      printf("Usage: daemon stop <directorio destino>\n");
      exit(EXIT_FAILURE);
    }
    return stop(argv[2]);
  } else {
    printf("Usage: daemon <start | stop> <...>\n");
    exit(EXIT_FAILURE);
  }
}
