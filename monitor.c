#include "monitor.h"

#include <errno.h>
#include <linux/limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/syslog.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

extern char *user_key;

void encrypt_file(const char *source_path, const char *destination_path,
                  const char *name) {

  char input_path[PATH_MAX] = {0};
  strcat(input_path, source_path);
  strcat(input_path, "/");
  strcat(input_path, name);

  FILE *ifp = fopen(input_path, "rb");
  if (ifp == NULL) {
    syslog(LOG_ERR, "Failed to open file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char output_path[PATH_MAX] = {0};
  strcat(output_path, destination_path);
  strcat(output_path, "/");
  strcat(output_path, name);

  FILE *ofp = fopen(output_path, "wb");
  if (ofp == NULL) {
    syslog(LOG_ERR, "Failed to open file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char byte;
  int key_index = 0;
  while (fread(&byte, 1, 1, ifp) == 1) {
    char new_byte = byte ^ user_key[key_index];
    fwrite(&new_byte, 1, 1, ofp);
    key_index = (key_index + 1) % strlen(user_key);
  }

  fclose(ifp);
  fclose(ofp);
}

void monitor_directory(const char *source_path, const char *destination_path) {
  int instance = inotify_init();

  if (instance < 0) {
    syslog(LOG_ERR, "Cannot create inotify instance: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  int watch_fd =
      inotify_add_watch(instance, source_path,
                        IN_CREATE | IN_MODIFY | IN_DELETE | IN_OPEN | IN_CLOSE);
  if (watch_fd == -1) {
    syslog(LOG_ERR, "Cannot watch '%s': %s\n", source_path, strerror(errno));
    exit(EXIT_FAILURE);
  }

  int poll_num;
  int n_fds = 1;
  struct pollfd fds[1];
  fds[0].fd = instance;
  fds[0].events = POLLIN;

  while (1) {
    poll_num = poll(fds, n_fds, -1);
    if (poll_num == -1) {
      if (errno == EINTR) {
        continue;
      }
      exit(EXIT_FAILURE);
    }
    if (fds[0].revents & POLLIN) {
      const struct inotify_event *event;
      ssize_t len;
      char buf[BUF_LEN];

      len = read(instance, buf, BUF_LEN);
      if (len < 0) {
        syslog(LOG_ERR, "Cannot read file descriptor for '%s': %s\n",
               source_path, strerror(errno));
        exit(EXIT_FAILURE);
      }

      for (char *ptr = buf; ptr < buf + len;
           ptr += sizeof(struct inotify_event) + event->len) {

        event = (const struct inotify_event *)ptr;

        if (event->len) {
          if (event->mask & IN_MODIFY) {
            encrypt_file(source_path, destination_path, event->name);
            syslog(LOG_NOTICE, "[%s] File '%s' encrypted\n", source_path,
                   event->name);
          }
          if (event->mask & IN_DELETE) {
            char path[PATH_MAX] = {0};
            strcat(path, destination_path);
            strcat(path, "/");
            strcat(path, event->name);
            remove(path);
            syslog(LOG_NOTICE, "[%s] File deleted: %s\n", path, event->name);
          }
        }
      }
    }
  }

  inotify_rm_watch(instance, watch_fd);
}
