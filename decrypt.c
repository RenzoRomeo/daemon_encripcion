#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ERROR(FMT, ...)                                                        \
  fprintf(stderr, FMT ": %s\n", ##__VA_ARGS__, strerror(errno))

void decrypt_file(const char *input_file_name,
                        const char *output_file_name, char *key) {
  FILE *input_file, *output_file;
  char ch;
  int key_index = 0;

  input_file = fopen(input_file_name, "r");
  if (input_file == NULL) {
    ERROR("Error opening input file '%s'", input_file_name);
    exit(EXIT_FAILURE);
  }

  output_file = fopen(output_file_name, "w");
  if (output_file == NULL) {
    ERROR("Error creating output file '%s'", output_file_name);
    fclose(input_file);
    exit(EXIT_FAILURE);
  }

  while ((ch = fgetc(input_file)) != EOF) {
    ch = ch ^ key[key_index];
    fputc(ch, output_file);
    key_index = (key_index + 1) % strlen(key);
  }

  printf("File %s decrypted successfully.\n", input_file_name);

  fclose(input_file);
  fclose(output_file);
}

void decrypt_directory(const char *directory_path, char *key) {
  DIR *dir;
  struct dirent *entry;

  dir = opendir(directory_path);
  if (dir == NULL) {
    ERROR("Error opening directory '%s'", directory_path);
    exit(EXIT_FAILURE);
  }

  char parent_dir[PATH_MAX];
  strncpy(parent_dir, directory_path, PATH_MAX);
  char *parent = dirname(parent_dir);

  char decrypted_dir[PATH_MAX];
  snprintf(decrypted_dir, PATH_MAX, "%s/decrypted_%s", parent,
           basename((char *)directory_path));
  if (mkdir(decrypted_dir, 0777) == -1) {
    if (errno != EEXIST) {
      ERROR("Error creating directory '%s'", decrypted_dir);
      exit(EXIT_FAILURE);
    }
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      char input_file[PATH_MAX];
      char output_file[PATH_MAX];

      snprintf(input_file, PATH_MAX, "%s/%s", directory_path, entry->d_name);
      snprintf(output_file, PATH_MAX, "%s/%s", decrypted_dir, entry->d_name);

      decrypt_file(input_file, output_file, key);
    }
  }

  closedir(dir);
}

int main(int argc, char *argv[]) {
  // TODO: arreglar warnings del compilador
  
  if (argc != 3) {
    printf("Usage: %s <directory_path> <encryption_key>\n", argv[0]);
    return 1;
  }

  char *directory_path = argv[1];
  char *key = argv[2];

  decrypt_directory(directory_path, key);

  return 0;
}
