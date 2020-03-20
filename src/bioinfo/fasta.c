#include <stdbool.h>

#include "fasta.h"
#include "new.h"
#include "xstring.h"
#include "strstream.h"


struct _fasta {
    strstream *str;
};


fasta *fasta_open(char *filename)
{
    fasta *ret = NEW(fasta);
    ret->str = strstream_open_file(filename);
    return ret;
}


bool fasta_has_next(fasta *self) 
{
    return false;
}


const xstring *fasta_next_xstr(fasta *self)
{
    return NULL;
}


void fasta_close(fasta *self)
{

}