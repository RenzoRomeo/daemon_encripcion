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

const char *pid_path = "/tmp/.daemon_pid";
char *user_key = NULL;

static pid_t destination_to_pid(const char *destination) {
  FILE *fp = fopen(pid_path, "rt");

  if (fp == NULL) {
    return -1;
  }

  char *dest_line = NULL;
  size_t bytes = 0;

  int found = 0;
  pid_t pid = -1;
  int bytes_read;
  while (!found && (bytes_read = getline(&dest_line, &bytes, fp)) > 0) {
    char *pid_line = NULL;
    dest_line[bytes_read - 1] = '\0';
    if (strcmp(dest_line, destination) == 0) {
      size_t b = 0;
      getline(&pid_line, &b, fp);
      sscanf(pid_line, "%d", &pid);
      found = 1;
      free(pid_line);
    } else {
      getline(&pid_line, 0, fp);
      free(pid_line);
      pid_line = NULL;
    }
    free(dest_line);
    bytes = 0;
    bytes_read = 0;
  }

  fclose(fp);

  return pid;
}

static void store_destination(const char *destination) {
  FILE *fp = fopen(pid_path, "a");
  if (fp == NULL) {
    syslog(LOG_ERR, "Could not open pid file at: %s", pid_path);
    printf("Could not open pid file at: %s", pid_path);
  }

  fprintf(fp, "%s\n%d\n", destination, getpid());
  fclose(fp);
}

static int delete_instance(const char *destination_path) {
  struct stat st;
  if (stat(pid_path, &st) == -1) {
    syslog(LOG_ERR, "Pid was not found at: %s", pid_path);
    printf("Pid was not found at: %s", pid_path);
  }

  FILE *fp = fopen(pid_path, "rb");

  if (fp == NULL) {
    syslog(LOG_ERR, "Could not open pid file at: %s", pid_path);
    printf("Could not open pid file at: %s", pid_path);
  }

  char *buffer = malloc(st.st_size);

  char *top = buffer;

  char *dest_line = NULL;
  size_t bytes = 0;
  int bytes_read = 0;

  while ((bytes_read = getline(&dest_line, &bytes, fp)) > 0) {
    char *pid_line = NULL;
    size_t b = 0;
    if (strlen(dest_line) - 1 == strlen(destination_path) &&
        strncmp(dest_line, destination_path, strlen(destination_path)) == 0) {
      getline(&pid_line, &b, fp);
      free(pid_line);
      pid_line = NULL;
      b = 0;
    } else {
      memcpy(top, dest_line, bytes_read);
      top += bytes_read;
      b = 0;
      bytes_read = getline(&pid_line, &b, fp);
      memcpy(top, pid_line, bytes_read);
      top += bytes_read;
      free(pid_line);
      pid_line = NULL;
      b = 0;
    }
    free(dest_line);
    bytes = 0;
    bytes_read = 0;
  }

  fclose(fp);

  fp = fopen(pid_path, "wb");
  if (fp == NULL) {
    syslog(LOG_ERR, "Could not open pid file at: %s", pid_path);
    printf("Could not open pid file at: %s", pid_path);
  }

  size_t size = top - buffer;

  if (fwrite(buffer, 1, size, fp) != size) {
    syslog(LOG_ERR, "Could write pid file at: '%s'", pid_path);
    printf("Could write pid file at: '%s'", pid_path);
  }

  fclose(fp);
  free(buffer);

  return -1;
}

static int start(const char *source_dir, const char *destination_dir,
                 const char *encryption_key) {
  char source_path[PATH_MAX];
  realpath(source_dir, source_path);

  char destination_path[PATH_MAX];
  realpath(destination_dir, destination_path);

  user_key = (char *)encryption_key;

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

  struct stat st_source = {0};
  if (stat(source_path, &st_source) == -1) {
    mkdir(source_path, 0700);
    syslog(LOG_NOTICE, "Created source directory: %s\n", source_path);
  }

  struct stat st_dest = {0};
  if (stat(destination_path, &st_dest) == -1) {
    mkdir(destination_path, 0700);
    syslog(LOG_NOTICE, "Created destination directory: %s\n", destination_path);
  }

  // TODO: pensar un nombre bueno
  const char *name = "File Enctryptor";
  daemonize(name);

  store_destination(destination_path);

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

  kill(pid, SIGKILL);

  delete_instance(destination_path);

  syslog(LOG_NOTICE, "Daemon instance stopped: %s\n", destination_path);
  printf("Daemon instance stopped: %s\n", destination_path);

  return 0;
}

int main(int argc, char *argv[]) {
  // TODO: agregar soporte para directorios anidados (?).
  if (argc < 2) {
    printf("Help message: %s\n", argv[0]);
    printf("Start instance: %s start <source directory> <destination "
           "directory> <encryption_key>\n",
           argv[0]);
    printf("Stop instance: %s stop <destination directory>\n", argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (strcmp(argv[1], "start") == 0) {
    if (argc != 5) {
      printf("Usage: %s start <source directory> <destination directory> <encryption key>\n",
             argv[0]);
      exit(EXIT_FAILURE);
    }
    const char *source_dir = argv[2];
    const char *destination_dir = argv[3];
    const char *encryption_key = argv[4];
    return start(source_dir, destination_dir, encryption_key);
  } else if (strcmp(argv[1], "stop") == 0) {
    if (argc != 3) {
      printf("Usage: %s stop <destination directory>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    return stop(argv[2]);
  } else {
    printf("Help message: %s\n", argv[0]);
    printf("Start instance: %s start <source directory> <destination "
           "directory> <encryption key>\n",
           argv[0]);
    printf("Stop instance: %s stop <destination directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}
