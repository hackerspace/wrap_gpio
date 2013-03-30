#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>

/* This is the offset (this is always the offset on the WRAP board) of the F0BAR0 register */
#define DATA  0xf400

void usage()
{
  fprintf(stderr,"usage: wrap_read_gpio [GPIO#]\n"
      "\tGPIO# = 32 to 37,and 39\n"
      "\nThese ports are present on the LPC connector.\n"
      "Read the WRAP board manual for details.\n" ) ;
  exit(0);
}

int main(int nargs, char **argv)
{
  unsigned int val;
  unsigned int gpio;

  if (nargs!=2)
    usage();

  gpio = atoi(argv[1]);
  if ( (gpio != 39) && (gpio<32 || gpio>37) ) {
    fprintf(stderr, "Error: invalid GPIO port (%s)\n", argv[1]);
    exit(0);
  }

  /* Request I/O prvileges */
  iopl(3);

  /*
   *   Offset 0x14 is GPIO-Data-In-1-Register.
   *   See table 6-30 on the SC1100 data sheet.
   *   This register contains 1 bit status for each GPIO in range 32 to 63
   */
  val = inl(DATA+0x14);

  /* Clear all bits except the GPIO bit that we need. */
  val = val & (1 << (gpio-32));
  printf("%c\n", (val!=0) ? '1' : '0');
  return 0;
}
