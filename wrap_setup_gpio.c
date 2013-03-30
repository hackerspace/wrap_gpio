/*
 * Note: See Page 50 of the SC1100 datasheet for PMR register info
 *   and Page 196 for GPIO information
 */
#include <sys/io.h>
#include <stdio.h>
#include <unistd.h>

#define GPIO_REGS 0xf400
int main(int nargs, char **argv) {
  unsigned int val;
  /*
   * The main requirement is to clear Bit14 in order to change
   * from the LPC config onto a straight GPIO config.  Other bits
   * are specified as "must be 0", so we clear them as well.
   * Bits 31-30, 26, 24-22, 20 and 15 should be written as '0';
   * Clear bit 14 disable the LPC, to use GPIO pins on the header
   * Remaining bits unchanged.
   * 0011 1010 0010 1111 0011 1111 1111 1111
   */
  unsigned int mask = 0x3a2f3fff;
  /* bit_ids is the GPIO bit number */
  unsigned char bit_ids[] = {   32,   33,   34,   35,   36,   37,  39 };
  /* Bits 6-4 are only for PME's, which we aren't using (so they don't
   * matter).  Bit 3 *must* be kept 0, so the only important ones are
   * bits 2-1-0.  We use the internal pull-ups for both the output pins
   * and the input pins.  Output must be enabled for all (otherwise we
   * can't disable the pull-ups on the inputs).
   *
   * bit_cfg b6 1=> debounce
   *     b5 0=> low level / falling edge
   *     b4 0=> edge trigger
   *     b3 0=> no lock
   *     b2 1=> internal pull-up (if b1:0 == 01)
   *     b1 0=> open drain
   *     b0 1=> output enabled
   *
   * 0x44 for input pins
   * 0x45 for output
   *                          32    33    34    35    36    37    39
   */
  unsigned char bit_cfg[] = { 0x44, 0x44, 0x45, 0x45, 0x45, 0x45, 0x45 };
  int ix;

  iopl(3);    /* Give us lots of privilege */
  val = inl(0x9030);  /* Fetch the PMR (32-bit fetch) */
  fprintf(stderr, "PMR register was %08x, setting it to %08x\n",
      val, val & mask);
  val = val & mask; /* Clear bits 31-24, 16, 14 */
  outl(val, 0x9030);  /* Write out the changed value */
  val = inl(0x9030);
  fprintf(stderr, "PMR register now %08x\n", val);

  /* Now set the GPIO configuration for our desired bits */
  val = inl(GPIO_REGS+0x18);  /* read existing interrupt enable bits */
  fprintf(stderr, "GPIO Int Enable1 was %08x, clearing our bits\n", val);
  val &= 0xffc0;    /* clear 37-32 */
  outl(val, GPIO_REGS+0x18);
  for (ix = 0; ix < sizeof(bit_ids); ix++) {
    fprintf(stderr, "Writing %08x to 0x20\n", (unsigned)bit_ids[ix]);
    outl((unsigned)bit_ids[ix], GPIO_REGS+0x20);
    fprintf(stderr, "Writing %08x to 0x24\n", (unsigned)bit_cfg[ix]);
    outl((unsigned)bit_cfg[ix], GPIO_REGS+0x24);
  }

  /* For the input pins, we need to write a '1'.  This will
   * assure that the pins are not driven low.  Since the pull-ups
   * are active, with no input there should be a HI, and if the
   * input is forced LO we will be able to sense it.
   *
   * For the output pins, we will also initialize all of the ports
   * to a HI (which should be what was previously present when the
   * port was configured as LPC).
   */
  val = inl(GPIO_REGS+0x10) & 0x3f;
  outl(val, GPIO_REGS+0x10);

  fprintf(stderr, "All done\n");
  return 0;
}
