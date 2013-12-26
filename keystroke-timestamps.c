#include <err.h>
#include <fcntl.h>
#include <glob.h>
#include <getopt.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <sys/select.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int print_usec = 0;
  char* device = strdup("/dev/input/by-path/platform-i8042-serio-0-event-kbd");
  struct option long_options[] = {
    {"device", required_argument, 0,            'd' },
    {"usec",   no_argument,       &print_usec,  1   },
    {0,        0,                 0,            0   },
  };
  while (1) {
    int c = getopt_long(argc, argv, "", long_options, NULL);
    if (c == -1) break;
    if (c == 'd') {
      free(device);
      device = strdup(optarg);
      continue;
    }
    if (c != 0) exit(EX_USAGE);
  }

  fd_set all_inputs;
  FD_ZERO(&all_inputs);
  int maxfd = 0;
  glob_t glob_result;
  int glob_return = glob(device, GLOB_ERR | GLOB_NOSORT, NULL, &glob_result);
  if (glob_return == GLOB_NOMATCH) {
    errx(EX_NOINPUT, "Could not find keyboard event file(s): %s", device);
  }
  if (glob_return != 0) {
    err(EX_NOINPUT, "Could not glob keyboard event file(s): %s", device);
  }
  free(device);
  for (unsigned i = 0; i < glob_result.gl_pathc; i++) {
    int open_fd = open(glob_result.gl_pathv[i], O_RDONLY);
    if (open_fd < 0) {
      err(EX_NOINPUT, "Could not open keyboard event file: %s", glob_result.gl_pathv[i]);
    }
    if (open_fd > maxfd) {
      maxfd = open_fd;
    }
    FD_SET(open_fd, &all_inputs);
  }
  globfree(&glob_result);

  struct input_event i;
  while (1) {
    fd_set ready_inputs = all_inputs;
    if (select(maxfd+1, &ready_inputs, NULL, NULL, NULL) != 1) {
      err(EX_IOERR, "select");
    }
    for (int fd=0; fd <= maxfd; fd++) {
      if (FD_ISSET(fd, &ready_inputs)) {
        int read_return = read(fd, &i, sizeof(i));
        if (read_return == -1) {
          err(EX_IOERR, "read");
        }
        if (read_return != sizeof(i)) {
          errx(EX_IOERR, "Unexpected EOF");
        }
        if (i.type == 1 && i.value == 1) {
          if (print_usec) {
            printf("%ld.%06ld\n", i.time.tv_sec, i.time.tv_usec);
          } else {
            printf("%ld\n", i.time.tv_sec);
          }
          fflush(stdout);
        }
      }
    }
  }
  return 0;
}
