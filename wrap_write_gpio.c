#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA  0xf410

void usage()
{
  fprintf(stderr,"usage: wrap_write_gpio GPIO_PIN VAL\n"
      "\tGPIO_PIN = 32 to 37,and 39\n"
      "\tVAL = 0 or 1\n"
      "\nThese ports are present on the LPC connector.\n"
      "Read the WRAP board manual for details.\n" ) ;
  exit(0);
}

int main(int nargs, char **argv)
{
  unsigned int val;
  unsigned int gpio;
  unsigned int new;

  if (nargs!=3)
    usage();

  gpio = atoi(argv[1]);
  new = atoi(argv[2]);

  if ( (gpio != 39) && (gpio<32 || gpio>37) ) {
    fprintf(stderr, "Error: invalid GPIO port (%s)\n", argv[1]);
    exit(0);
  }
  if ( new > 1 ) {
    fprintf(stderr, "Error: VAL not in {0,1}: (%s)\n", argv[2]);
    exit(0);
  }

  /* Request I/O prvileges */
  iopl(3);

  val = inl(DATA+0x4);

  /* Clear all bits except the GPIO bit that we need. */
  if(new) {
    fprintf(stderr, "on\n");
    val = val | (1 << (gpio-32));
//printf("%c\n", (val!=0) ? '1' : '0');
  } else {
    fprintf(stderr, "off\n");
    val = val & ~(1 << (gpio-32));
  }
  outl(val, DATA);
  return 0;
}
