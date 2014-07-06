#include <err.h>
#include <fcntl.h>
#include <glob.h>
#include <getopt.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <sys/select.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int print_usec = 0;
  char* device = "/dev/input/by-path/*kbd*";
  char* output = NULL;
  struct option long_options[] = {
    {"device", required_argument, 0,            'd' },
    {"output", required_argument, 0,            'o' },
    {"usec",   no_argument,       &print_usec,  1   },
    {0,        0,                 0,            0   },
  };
  while (1) {
    int c = getopt_long(argc, argv, "", long_options, NULL);
    if (c == -1) break;
    if (c == 'd') {
      device = optarg;
      continue;
    }
    if (c == 'o') {
      output = optarg;
      continue;
    }
    if (c != 0) exit(EX_USAGE);
  }

  /* Open output file */
  FILE *output_file;
  if (output == NULL) {
    output_file = stdout;
  } else {
    output_file = fopen(output, "a");
    if (output_file == NULL) {
      err(EX_CANTCREAT, "Could not open output file: %s", output);
    }
  }

  /* Open device inputs */
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
  for (unsigned i = 0; i < glob_result.gl_pathc; i++) {
    int device_fd = open(glob_result.gl_pathv[i], O_RDONLY);
    if (device_fd < 0) {
      err(EX_NOINPUT, "Could not open keyboard event file: %s", glob_result.gl_pathv[i]);
    }
    if (device_fd > maxfd) {
      maxfd = device_fd;
    }
    FD_SET(device_fd, &all_inputs);
  }
  globfree(&glob_result);

  /* Report keystroke timestamps */
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
            fprintf(output_file, "%ld.%06ld\n", i.time.tv_sec, i.time.tv_usec);
          } else {
            fprintf(output_file, "%ld\n", i.time.tv_sec);
          }
          fflush(output_file);
        }
      }
    }
  }
  return 0;
}
