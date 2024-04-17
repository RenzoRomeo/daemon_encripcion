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

void monitor_directory(const char *source_path, const char *destiny_path) {
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
          if (event->mask & IN_CREATE) {
            syslog(LOG_NOTICE, "[%s] File created: %s\n", source_path,
                   event->name);
          }
          if (event->mask & IN_MODIFY) {
            syslog(LOG_NOTICE, "[%s] File modified: %s\n", source_path,
                   event->name);
          }
          if (event->mask & IN_DELETE) {
            syslog(LOG_NOTICE, "[%s] File deleted: %s\n", source_path,
                   event->name);
          }
        }
      }
    }
  }

  inotify_rm_watch(instance, watch_fd);
}
