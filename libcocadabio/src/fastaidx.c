/*
 * COCADA - COCADA Collection of Algorithms and DAta Structures
 *
 * Copyright (C) 2016  Paulo G S Fonseca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include "fastaidx.h"
#include "cstrutil.h"
#include "new.h"
#include "vec.h"


struct _FASTAIndex {
	char *path;
	Vec *dscs;
	Vec *seqs;
};


FASTAIndex *FASTAIndex_new(const char *src_path)
{
	FASTAIndex *ret = NEW(FASTAIndex);
	ret->path = cstr_clone(src_path);
	ret->dscs = vec_new(sizeof(size_t));
	ret->seqs = vec_new(sizeof(size_t));
	return ret;
}


void FASTAIndex_finalise(void *ptr, const Finaliser *fnr)
{
	FASTAIndex *self = (FASTAIndex *)ptr;
	FREE(self->path);
	DESTROY_FLAT(self->dscs, vec);
	DESTROY_FLAT(self->seqs, vec);
}


void fasta_index_free(FASTAIndex *self)
{
	DESTROY_FLAT(self, FASTAIndex);
}


size_t fasta_index_size(FASTAIndex *self)
{
	return vec_len(self->dscs);
}


void fasta_index_add(FASTAIndex *self, size_t dsc_offset, size_t seq_offset)
{
	vec_push_size_t(self->dscs, dsc_offset);
	vec_push_size_t(self->seqs, seq_offset);
}


FASTARecOffsets fasta_index_get(FASTAIndex *self, size_t rec_no)
{
	FASTARecOffsets ret;
	ret.descr_offset = vec_get_size_t(self->dscs, rec_no);
	ret.seq_offset = vec_get_size_t(self->seqs, rec_no);
	return ret;
}
