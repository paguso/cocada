#include <assert.h>
#include <string.h>

#include "cstrutil.h"
#include "new.h"
#include "strview.h"


void strview_init(strview *self, const char *src, size_t len)
{
	self->str = src;
	self->len = len;
}


strview *strview_new(const char* src, size_t len)
{
	strview *ret = NEW(strview);
	strview_init(ret, src, len);
	return ret;
}

void strview_init_from_range(strview *self, const char *src, size_t from, size_t to)
{
	assert (from <= to);
	self->str = src + from;
	self->len = to - from;
}


strview* strview_new_from_range(const char *src, size_t from, size_t to)
{
	assert (from <= to);
	strview *ret = NEW(strview);
	strview_init_from_range(ret, src, from, to);
	return ret;
}

char *strview_to_new_cstr(strview *self)
{
	char *ret = cstr_new(self->len);
	strview_cpy_to_cstr(self, ret);
	return ret;
}

void strview_cpy_to_cstr(strview *self, char *dest)
{
	strncpy(dest, self->str, self->len);
}

size_t strview_len(strview *self)
{
	return self->len;
}

bool strview_eq_cstr(strview *self, const char *other)
{
	return strview_eqn_cstr(self, other, strlen(other));
}

bool strview_eqn_cstr(strview *self, const char *other, size_t n)
{
	if (self->len != n) return false;
	for (size_t i = 0 ; i < n; i++) {
		if (self->str[i] != other[i]) {
			return false;
		}
	}
	return true;
}




typedef struct {
	int n;
	int *delta[256];
} fsm;


static void fsm_free(fsm *f)
{
	for (size_t i = 0; i < 256; free(f->delta[i++]));
	free(f);
}


static fsm *build_fsm(const char *pat, int len)
{
	assert( 0 <= len );
	fsm *ret = NEW(fsm);
	ret->n = len + 1;
	int fail = 0;
	int m = 256; // 1-byte char assumed
	for (int i = 0; i < m; i++) {
		ret->delta[i] = calloc(ret->n, sizeof(int));
	}
	for (int j = 0 ; j < len; j++) {
		for (int i = 0; i < m; i++) {
			ret->delta[i][j] = ret->delta[i][fail];
		}
		ret->delta[(size_t)pat[j]][j] = j + 1;
		fail = ret->delta[(size_t)pat[j]][fail];
	}
	for (int i = 0; i < m; i++) {
		ret->delta[i][ret->n - 1] = ret->delta[i][0];
	}
	return ret;
}



typedef enum {
	TOKENIZER = 0
} strview_iter_type;

typedef struct {
	char *sep;
	size_t sep_len;
	size_t pos;
} tok_data_t;


typedef struct {
	bool (*has_next) (strview_iter *it);
	strview (*next) (strview_iter *it);
} strview_iter_vt;


struct strview_iter {
	strview *src;
	strview_iter_vt *vt;
	strview_iter_type type;
	union {
		tok_data_t tok_data;
	} aux;
};


void strview_iter_free(strview_iter *self)
{
	if (!self) return;
	switch (self->type) {
	case TOKENIZER:
		FREE(self->aux.tok_data.sep);
		break;
	default:
		break;
	}
	FREE(self);
}


bool strview_iter_has_next(strview_iter *it)
{
	return it->vt->has_next(it);
}


strview strview_iter_next(strview_iter *it)
{
	return it->vt->next(it);
}



static bool _strview_tok_has_next(strview_iter *it)
{
	size_t pos = it->aux.tok_data.pos;
	size_t len = strview_len(it->src);
	return pos <= len;
}

static strview _strview_tok_next(strview_iter *it)
{
	size_t pos = it->aux.tok_data.pos;
	const char *str = it->src->str;
	const char *sep = it->aux.tok_data.sep;
	size_t sep_len = it->aux.tok_data.sep_len;
	size_t len = strview_len(it->src);
	// find next occurrence of separator
	size_t sep_pos = pos;
	while (sep_pos < len) {
		size_t j;
		for (j = 0; j < sep_len && sep_pos + j < len && sep[j] == str[sep_pos + j]; j++);
		if (j == sep_len)
			break;
		sep_pos++;
	}
	if (sep_pos < len) { // found separator at sep_pos
		strview ret = {.str = str + pos, .len = sep_pos - pos };
		it->aux.tok_data.pos = sep_pos + sep_len;
		return ret;
	} else { // separator not found
		strview ret = {.str = str + pos, .len = len - pos };
		it->aux.tok_data.pos = len + 1;
		return ret;
	}
}


static strview_iter_vt _strview_tok_vt = {_strview_tok_has_next, _strview_tok_next};


strview_iter *strview_tok(strview *self, const char *sep, size_t sep_len)
{
	strview_iter *ret = NEW(strview_iter);
	ret->type = TOKENIZER;
	ret->src = self;
	ret->aux.tok_data = (tok_data_t) {
		.sep = cstr_clone_len(sep, sep_len), .sep_len = sep_len, .pos = 0
	};
	ret->vt = &_strview_tok_vt;
	return ret;
}


