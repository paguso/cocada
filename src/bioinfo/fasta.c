#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cstringutil.h"
#include "fasta.h"
#include "new.h"
#include "strread.h"
#include "trait.h"

typedef struct _fastaread {
    strread _t_strread;
    FILE *src;
    size_t file_pos;
} fastaread;


IMPL_TRAIT(fastaread, strread)


static void _reset(void *self)
{   
    fastaread *fr = (fastaread *)self;
    fseek(fr->src, fr->file_pos, SEEK_SET);
}


static xchar_t _getc(void *self)
{
    fastaread *fr = (fastaread *)self;
    int c;
    while(true) {
        c = fgetc(fr->src);
        if (c=='>') {
            ungetc(c, fr->src);
            c = EOF;
            break;
        }
        else if (c=='\n'|| c=='\r') {
            continue;
        }
        else {
            break;
        }
    }
    return c;
}


static size_t _read_str(void *self, char *dest, size_t n)
{
    FILE *src = ((fastaread *)self)->src;
    char *origdest = dest;
    memset(dest, '\0', n+1);
    while( !feof(src) && n > 0 ) {
        fgets(dest, n+1, src);
        size_t l = strlen(dest);
        if (dest[0] == '>') {
            memset(dest, '\0', l );
            break;
        }
        while (l > 0 && ( dest[l-1]=='\n' || dest[l-1]=='\r' )) {
            dest[l-1] = '\0';
            l--;
        }
        dest += l;
        n -= l;
    }
    return dest-origdest;
}


static size_t _read_str_until(void *self, char *dest, char delim)
{
    FILE *src = ((fastaread *)self)->src;
    size_t nread = 0;
    char c;
    while( !feof(src) ) {
        c = fgetc(src);
        if ( c == delim || c == '>' ) {
            ungetc(c, src);
            break;
        }
        else if ( c == '\n' || c=='\r' ) {
            continue;
        }
        dest[nread++] = c;
    }
    dest[nread] = '\0';
    return nread;
}



static size_t _sizeof_char(void *self)
{
    return sizeof(char);
}



static strread_vt _strread_vt  = 
{
    .getc = _getc,
    .read_str = _read_str,
    .read_str_until = _read_str_until,
    .reset = _reset,
    .sizeof_char = _sizeof_char,
};


static void _fastaread_init(fastaread *ret, FILE *src) {
    ret->src = src;
    ret->_t_strread.impltor = ret;
    ret->_t_strread.vtbl = &_strread_vt;
}



struct _fasta {
    FILE *src;
    fasta_record cur_rec;
    fastaread rd;
    fasta_record_reader cur_rec_rd;
};


fasta *fasta_open(char *filename)
{
    fasta *ret = NEW(fasta);
    ret->src = fopen(filename, "r");
    _fastaread_init(&(ret->rd), ret->src);
    ret->cur_rec_rd.descr = cstr_new(100);
    ret->cur_rec_rd.seqread = fastaread_strread(&(ret->rd));
    return ret;
}


static bool _goto_next(fasta *self) {
    int c;
    while(!feof(self->src)) {
        if ((c=fgetc(self->src)) == '>') {
            ungetc(c, self->src);
            return true;
        }
    }
    return false;
}



bool fasta_has_next(fasta *self) 
{
    long cur = ftell(self->src);
    bool ret = _goto_next(self);
    fseek(self->src, cur, SEEK_SET);
    return ret;
}



fasta_record_reader *fasta_next_reader(fasta *self)
{
    if (!_goto_next(self)) {
        return NULL;
    }
    assert(fgetc(self->src) == '>');
    cstr_clear(self->cur_rec_rd.descr, strlen(self->cur_rec_rd.descr));
    fgets(self->cur_rec_rd.descr, 100, self->src);
    size_t l = strlen(self->cur_rec_rd.descr);
    while ( self->cur_rec_rd.descr[l-1] != '\n' ) {
        self->cur_rec_rd.descr = cstr_resize(self->cur_rec_rd.descr, l + 50);
        fgets(self->cur_rec_rd.descr + l, 50, self->src);
        l = strlen(self->cur_rec_rd.descr);
    }
    self->cur_rec_rd.descr[l-1] = '\0';
    self->rd.file_pos = ftell(self->src);
    return &(self->cur_rec_rd);
}


void fasta_close(fasta *self)
{
    fclose(self->src);
    FREE(self->cur_rec.descr);
    FREE(self->cur_rec_rd.descr);
}