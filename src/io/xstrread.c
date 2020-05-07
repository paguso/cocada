
#include "xstrread.h"


void xstrread_reset(xstrread *trait)
{
	trait->vt->reset(trait);
}


xchar_wt xstrread_getc(xstrread *trait)
{
	return trait->vt->getch(trait);
}


size_t xstrread_read(xstrread *trait, xstr *dest, size_t n)
{
	return trait->vt->read(trait, dest, n);
}


size_t xstrread_read_until(xstrread *trait,  xstr *dest, xchar_t delim)
{
	return trait->vt->read_until(trait, dest, delim);
}



