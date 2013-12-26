#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int print_usec = 0;
  struct option long_options[] = {
    {"usec",   no_argument,       &print_usec,  1   },
    {0,        0,                 0,            0   },
  };
  while (1) {
    int c = getopt_long(argc, argv, "", long_options, NULL);
    if (c == -1) break;
    if (c != 0) exit(EX_USAGE);
  }

  int fd = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY);
  if (fd < 0) {
    err(EX_NOINPUT, "Could not open keyboard event file");
  }
  struct input_event i;
  while (read(fd, &i, sizeof(i)) == sizeof(i)) {
    if (i.type == 1 && i.value == 1) {
      if (print_usec) {
        printf("%ld.%ld\n", i.time.tv_sec, i.time.tv_usec);
      } else {
        printf("%ld\n", i.time.tv_sec);
      }
      fflush(stdout);
    }
  }
  return 0;
}
