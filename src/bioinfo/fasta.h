#include <stdbool.h>

#include "strread.h"


typedef struct _fasta fasta;

typedef struct {
    char *descr;
    char *seq;    
} fasta_record;

typedef struct {
    char *descr;
    strread *seqread;
} fasta_record_reader;

fasta *fasta_open(char *filename);

bool fasta_has_next(fasta *self);

fasta_record_reader *fasta_next_reader(fasta *self);


void fasta_close(fasta *self);