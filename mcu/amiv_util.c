#include "amiv_main.h"
#include "amiv_util.h"

#define INT_DIGITS 19		/* enough for 64 bit integer */

uint32_t AMIV_UTIL_strlen(char *p, char end)
{
	uint32_t i = 0;
	while(*p != end)
	{
		i++;
		p++;
	}
	return i;
}

char *AMIV_UTIL_itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

char * AMIV_UTIL_itoahex(int i, int chars)
{
    unsigned char n;
    static char buf[INT_DIGITS + 2];
    char *s = buf + INT_DIGITS + 1;	/* points to terminating '\0' */

    s += 4;
    *s = '\0';

    for (n = chars; n != 0; --n) {
        *--s = "0123456789ABCDEF"[i & 0x0F];
        i >>= 4;
    }
    s--;
    *s = 'x';
    s--;
    *s = '0';
    return s;
}

int32_t AMIV_UTIL_atoi(char *p)
{
	uint32_t k = 0;
	while (*p)
	{
		k = (k<<3)+(k<<1)+(*p)-'0';
		p++;
	}
	return k;
}

int AMIV_UTIL_atox(char *p)
{
	int c = 1;
	int n;
	int i;
	int res = 0;
	for (i = AMIV_UTIL_strlen(p, '\0') - 1; i >= 0; i--)
	{
		n = p[i] - '0';
		if (n > 9) n -= ((n >= 'a'-'0')? 'a': 'A') - '0' - 10;

		res += n*c;
		c *= 16;
	}
	return res;
}

void AMIV_UTIL_memset(uint8_t *src, uint8_t value, uint32_t size)
{
	uint32_t i;

	for(i = 0; i < size; i++)
	{
		*src++ = value;
	}
}

