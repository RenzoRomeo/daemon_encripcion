#include <linux/limits.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

void watch_directory(const char *source_path, const char *destiny_path) {
  int instance = inotify_init();

  if (instance < 0) {
    // TODO: manejar error
  }

  int watch_fd =
      inotify_add_watch(instance, source_path, IN_CREATE | IN_MODIFY | IN_DELETE);

  if (watch_fd < 0) {
    // TODO: manejar error
  }

  size_t buffer_size = sizeof(struct inotify_event) + PATH_MAX + 1;
  struct inotify_event *event = (struct inotify_event *)malloc(buffer_size);

  while (read(watch_fd, event, buffer_size)) {
    // TODO: manejar eventos
  }

  free(event);

  inotify_rm_watch(instance, watch_fd);
}
