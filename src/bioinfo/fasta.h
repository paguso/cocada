#include <stdbool.h>

#include "xstring.h"

typedef struct _fasta fasta;

typedef struct {
    char *descr;
    xstring *seq;    
} fasta_xstring;

fasta *fasta_open(char *filename);

bool fasta_has_next(fasta *self);

const xstring *fasta_next_xstr(fasta *self);

void fasta_close(fasta *self);