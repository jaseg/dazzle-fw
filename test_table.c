#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#define TOP2BITS(x) ((x & (3L << (32-2))) >> (32-2))

uint32_t usqrt(uint32_t x)
{
      uint32_t a = 0L;                   /* accumulator      */
      uint32_t r = 0L;                   /* remainder        */
      uint32_t e = 0L;                   /* trial product    */

      int i;

      for (i = 0; i < 32; i++)   /* NOTE 1 */
      {
            r = (r << 2) + TOP2BITS(x); x <<= 2; /* NOTE 2 */
            a <<= 1;
            e = (a << 1) + 1;
            if (r >= e)
            {
                  r -= e;
                  a++;
            }
      }
      return a;
}

int main(void) {
    for (int v=0; v<0x10000; v++) {
        uint64_t sqr = usqrt(v);
        uint64_t ssqr = usqrt(sqr>>16);
        uint64_t vsq = ((uint32_t)v)*((uint32_t)v);
        uint64_t out = (((vsq * sqr) >> 24) * ssqr)>>20;
        printf("%lu %lu\n", (uint32_t)out, usqrt(v<<16));
    }
}
