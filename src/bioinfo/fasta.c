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
    size_t cur_rec_len[2];
    fastaread rd;
    fasta_record_reader cur_rec_rd;
    size_t cur_rec_rd_len[2];
};


fasta *fasta_open(char *filename)
{
    fasta *ret = NEW(fasta);
    ret->src = fopen(filename, "r");
    _fastaread_init(&(ret->rd), ret->src);
    ret->cur_rec_len[0] = ret->cur_rec_len[1] = 100;
    ret->cur_rec_rd_len[0] = ret->cur_rec_rd_len[1] = 100;
    ret->cur_rec.descr = cstr_new(ret->cur_rec_len[0]);
    ret->cur_rec.seq = cstr_new(ret->cur_rec_len[1]);
    ret->cur_rec_rd.descr = cstr_new(ret->cur_rec_rd_len[0]);
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


const fasta_record *fasta_next(fasta *self)
{
    if (!_goto_next(self)) {
        return NULL;
    }
    // load description
    assert(fgetc(self->src) == '>');
    cstr_clear(self->cur_rec.descr, self->cur_rec_len[0]);
    size_t l = 0;
    bool eol = false;
    while(!eol) {
        if (self->cur_rec_len[0] == l) {
            self->cur_rec_len[0] *= 1.66f;
            //(size_t)(1.66f * self->cur_rec_len[0]);
            self->cur_rec.descr = cstr_resize(self->cur_rec.descr, self->cur_rec_len[0]);
        }
        fgets(self->cur_rec.descr + l, self->cur_rec_len[0] - l + 1, self->src);
        l = strlen(self->cur_rec.descr);
        while (self->cur_rec.descr[l-1]=='\n' || self->cur_rec.descr[l-1]=='\r') {
            self->cur_rec.descr[--l] = '\0';
            eol = true;
        }
    }
    // load sequence
    cstr_clear(self->cur_rec.seq, self->cur_rec_len[1]);
    l = 0;
    while( !feof(self->src) ) {
        if (self->cur_rec_len[1] == l) {
           self->cur_rec_len[1] *= 1.66f;
           //(size_t)(1.66f * self->cur_rec_len[1]);
            self->cur_rec.seq = cstr_resize(self->cur_rec.seq, self->cur_rec_len[1]);
        }
        fgets(self->cur_rec.seq + l, self->cur_rec_len[1] - l + 1, self->src);
        if (self->cur_rec.seq[l] == '>') {
            fseek(self->src, l - strlen(self->cur_rec.seq), SEEK_CUR);
            self->cur_rec.seq[l] = '\0';
            break; 
        }
        l = strlen(self->cur_rec.seq);
        while (self->cur_rec.seq[l-1]=='\n' || self->cur_rec.seq[l-1]=='\r') {
            self->cur_rec.seq[--l] = '\0';
        }
    } 
    return &(self->cur_rec);
}


const fasta_record_reader *fasta_next_reader(fasta *self)
{
    if (!_goto_next(self)) {
        return NULL;
    }
    // load description
    assert(fgetc(self->src) == '>');
    cstr_clear(self->cur_rec_rd.descr, self->cur_rec_rd_len[0]);
    size_t l = 0;
    bool eol = false;
    while(!eol) {
        if (self->cur_rec_rd_len[0] == l) {
            self->cur_rec_rd_len[0] *= 1.66f;
            //(size_t)(1.66f * self->cur_rec_rd_len[0]);
            self->cur_rec_rd.descr = cstr_resize(self->cur_rec_rd.descr, self->cur_rec_rd_len[0]);
        }
        fgets(self->cur_rec_rd.descr + l, self->cur_rec_rd_len[0] - l + 1, self->src);
        l = strlen(self->cur_rec_rd.descr);
        while (self->cur_rec_rd.descr[l-1]=='\n' || self->cur_rec_rd.descr[l-1]=='\r') {
            self->cur_rec_rd.descr[--l] = '\0';
            eol = true;
        }
    }
    // load sequence reader
    self->rd.file_pos = ftell(self->src);
    return &(self->cur_rec_rd);
}


void fasta_close(fasta *self)
{
    fclose(self->src);
    FREE(self->cur_rec.descr);
    FREE(self->cur_rec_rd.descr);
}