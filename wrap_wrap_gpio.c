#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* This is the offset (this is always the offset on the WRAP board) of the F0BAR0 register */
#define DATA  0xf400

#define CMD_LEN 100
#define MIN_PIN 32
#define MAX_PIN 37

char *cmd, *line, *param;
unsigned int val, oldval;

int stdin_ready() {
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO (&fds);
  FD_SET (0, &fds);  //STDIN
  select (1, &fds, NULL, NULL, &tv);  // number of fds+1
  return FD_ISSET (0, &fds);
}

size_t get_line(char *s, size_t n, FILE * f) {
  char *p = fgets(s, n, f);
  if (p == NULL) return -1;

  size_t len = strlen(s);
  if (s[len-1] == '\n') {
    s[--len] = '\0';
  }
  return len;
}

int check_pin(int pin_number) {
  return (pin_number >= MIN_PIN &&
          pin_number <= MAX_PIN &&
          pin_number != 36);
}
int get_byte() {
  param = strtok(NULL, " ");
  if(param == NULL) {
    fprintf(stderr, "missing byte parameter\n");
    return -1;
  }
  return atoi(param);
}

int get_pin_param() {
  param = strtok(NULL, " ");
  if(param == NULL) {
    fprintf(stderr, "missing pin parameter\n");
    return -1;
  }
  int pin = atoi(param);
  if(!check_pin(pin)) {
    fprintf(stderr, "invalid pin: %d\n");
    return -1;
  }
  return pin;
}

int in(int pin) {
  // 1 high, 0 low
  int res = val & (1 << (pin-32));
  return res!=0;
}

void on(int pin) {
  val = val | (1 << (pin-32));
  outl(val, DATA+0x10);
}

void off(int pin) {
  val = val & ~(1 << (pin-32));
  outl(val, DATA+0x10);
}

void pulse(int pin) {
  on(pin);
  off(pin);
}

void shift(int data_pin, int clock_pin, int latch_pin,
           unsigned int data) {

  unsigned int i = 0;
  for(; i < 8; i++) {
    off(data_pin);
    if(data & (1 << (7-i))) {
      on(data_pin);
    }
    pulse(clock_pin);
  }
  pulse(latch_pin);
}

int main (int nargs, char **argv) {
  int data, pin, data_pin, clock_pin, latch_pin;
  unsigned int watch_mask;
  int len, err;

  /* Request I/O privileges */
  iopl (3);

  /* Buffers */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IOLBF, 0);

  /*
   *   Offset 0x14 is GPIO-Data-In-1-Register.
   *   See table 6-30 on the SC1100 data sheet.
   *   This register contains 1 bit status for each GPIO in range 32 to 63
   */
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 1000000;

  line = malloc(CMD_LEN);
  oldval = inl(DATA + 0x14);
  for (;;) {
    if (stdin_ready() != 0) {
      len = get_line(line, CMD_LEN, stdin);
      fprintf(stderr, "in: '%s' length: %d\n", line, len);

      if(len < 0) break; // EOF
      if(!len) continue; // EMPTY

      cmd = strtok(line, " ");
      if(strncmp(cmd, "IN", CMD_LEN) == 0) {
        pin = get_pin_param();
        if(pin > 0) {
          printf("S %d %d\n", pin, in(pin));
        }
      }
      if(strncmp(cmd, "ON", CMD_LEN) == 0) {
        pin = get_pin_param();
        if(pin > 0) {
          on(pin);
        }
      }
      if(strncmp(cmd, "OFF", CMD_LEN) == 0) {
        pin = get_pin_param();
        if(pin > 0) {
          off(pin);
        }
      }
      if(strncmp(cmd, "SHIFT", CMD_LEN) == 0) {
        data_pin = get_pin_param();
        clock_pin = get_pin_param();
        latch_pin = get_pin_param();
        data = get_byte();
        if(data_pin > 0 && clock_pin > 0 && latch_pin > 0) {
          shift(data_pin, clock_pin, latch_pin, data);
        }
      }
      if(strncmp(cmd, "WATCH", CMD_LEN) == 0) {
        pin = get_pin_param();
        watch_mask |= 1 << (pin-32);
      }

      if(strncmp(cmd, "UNWATCH", CMD_LEN) == 0) {
        pin = get_pin_param();
        watch_mask &= ~(1 << (pin-32));
      }

    }

    /* check states */
    val = inl(DATA + 0x14);
    if (oldval != val) {
      pin = (oldval ^ val) & watch_mask;
      if(pin) {
        printf("IN%x\n", pin);
      }
      oldval = val;
    }
    nanosleep (&ts, NULL);
  }
}
