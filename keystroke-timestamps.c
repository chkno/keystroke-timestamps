#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>

int main ()
{
  struct input_event i;
  int fd = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY);
  while (fd && read(fd, &i, sizeof(i)) == sizeof(i)) {
    if (i.type == 1 && i.value == 1) {
      printf("%ld.%ld\n", i.time.tv_sec, i.time.tv_usec);
      fflush(stdout);
    }
  }
  return 0;
}
