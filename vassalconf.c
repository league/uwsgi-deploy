#include <assert.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int show_help();
int test(char* name, char* version);
int uninstall(char* name, char* version);
int install(char* name, char* version);

#define ETC_DIR "/etc/uwsgi/vassals"
#define SOCK_DIR "/var/run/uwsgi"
#define BUF_SIZE 16384

int main(int argc, char** argv)
{
  srand(time(NULL));
  uid_t e = geteuid();
  assert(e == 0); // Must be root
  if(argc < 4) {
    return show_help();
  }
  else {
    char* cmd = argv[1];
    char* name = argv[2];
    char* version = argv[3];
    if(strcmp(cmd, "test") == 0) {
      return test(name, version);
    }
    else if(strcmp(cmd, "rm") == 0) {
      return uninstall(name, version);
    }
    else if(strcmp(cmd, "install") == 0) {
      return install(name, version);
    }
    else {
      return show_help();
    }
  }
}

int show_help()
{
  fprintf(stderr, "Usage: vassalconf {test|install|rm} PROJECT VERSION\n");
  return 0;
}

void assert_no_slashes(char* s)
{
  while(*s) {
    if(*s == '/') {
      fprintf(stderr, "Error: cannot contain slashes\n");
      exit(1);
    }
    s++;
  }
}

void format_ini_pathname(char* buf, char* name, char* version)
{
  assert_no_slashes(name);
  assert_no_slashes(version);
  int k = snprintf(buf, PATH_MAX, "%s/%s-%s.ini", ETC_DIR, name, version);
  assert(k > 0 && k < PATH_MAX);
}

char* format_socket_pathnames(char* sock_buf, char* link_buf,
                              char* name, char* version)
{
  int k;
  k = snprintf(sock_buf, PATH_MAX, "%s/%s-%s.socket",
               SOCK_DIR, name, version);
  assert(k > 0 && k < PATH_MAX);
  k = snprintf(link_buf, PATH_MAX, "%s/%s.socket", SOCK_DIR, name);
  assert(k > 0 && k < PATH_MAX);
  return strrchr(sock_buf, '/') + 1;
}

FILE* copy_ini_from_stdin(char* name, char* version)
{
  char ini_path_buf[PATH_MAX];
  format_ini_pathname(ini_path_buf, name, version);
  fprintf(stderr, "Writing %s\n", ini_path_buf);
  FILE* ini = fopen(ini_path_buf, "w");
  assert(ini);
  char buf[BUF_SIZE];
  int k = fread(buf, 1, BUF_SIZE, stdin);
  while(k > 0) {
    int j = fwrite(buf, 1, k, ini);
    assert(j == k);
    k = fread(buf, 1, BUF_SIZE, stdin);
  }
  return ini;
}

int test(char* name, char* version)
{
  FILE* ini = copy_ini_from_stdin(name, version);
  int port = rand() % 700 + 8100;
  fprintf(ini, "\nsocket = localhost:%d\nprotocol = http\n", port);
  fclose(ini);
  printf("http://localhost:%d/ping\n", port);
  return 0;
}

void symlink_or_abort(const char* oldpath, const char* newpath)
{
  fprintf(stderr, "Linking %s -> %s\n", newpath, oldpath);
  int r = symlink(oldpath, newpath);
  if(r != 0) {
    perror("Error");
    exit(2);
  }
}

void wait_for(char* path)
{
  int s = 1;
  struct stat st;
  int r = stat(path, &st);
  while(r < 0 && errno == ENOENT) {
    if(s > 10) {
      fprintf(stderr, "Timeout waiting for %s...\n", path);
      exit(8);
    }
    fprintf(stderr, "Waiting for %s...\n", path);
    sleep(s++);
    r = stat(path, &st);
  }
}

int install(char* name, char* version)
{
  char sock_path_buf[PATH_MAX], link_path_buf[PATH_MAX];
  char* basename = format_socket_pathnames(sock_path_buf, link_path_buf,
                                           name, version);

  FILE* ini = copy_ini_from_stdin(name, version);
  fprintf(ini, "\nsocket = %s\n", sock_path_buf);
  fclose(ini);

  // Read existing symlink
  char prev_path_buf[PATH_MAX];
  int r = readlink(link_path_buf, prev_path_buf, PATH_MAX);
  if(r <= 0) {
    symlink_or_abort(basename, link_path_buf);
  }
  else if(strcmp(prev_path_buf, basename) != 0) {
    wait_for(sock_path_buf);
    unlink(link_path_buf);
    symlink_or_abort(basename, link_path_buf);
    // deactivate previously running vassal
    int k = strlen(name);
    char* ext = strrchr(prev_path_buf, '.');
    if(strncmp(basename, name, k) == 0 && strlen(basename) > k
       && basename[k] == '-' && ext)
      {
        char old_version[PATH_MAX] = { 0 };
        strncpy(old_version, prev_path_buf+k+1, ext-prev_path_buf-k-1);
        uninstall(name, old_version);
      }
  }
  return 0;
}

int uninstall(char* name, char* version)
{
  // Ensure we're not the target of the symlink
  char sock_path_buf[PATH_MAX], link_path_buf[PATH_MAX];
  char* basename = format_socket_pathnames(sock_path_buf, link_path_buf,
                                           name, version);

  char prev_path_buf[PATH_MAX];
  int r = readlink(link_path_buf, prev_path_buf, PATH_MAX);
  if(r > 0 && strcmp(prev_path_buf, basename) == 0) {
    fprintf(stderr, "Error: cannot remove active vassal.\n");
    exit(5);
  }

  char ini_path_buf[PATH_MAX];
  format_ini_pathname(ini_path_buf, name, version);
  fprintf(stderr, "Removing %s\n", ini_path_buf);
  r = unlink(ini_path_buf);
  if(r < 0) {                   /* continue on unlink error */
    perror("Warning");
  }
  return 0;
}

