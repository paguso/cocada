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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitarr.h"
#include "bitbyte.h"
#include "bytearr.h"
#include "csrsbitarray.h"
#include "mathutil.h"
#include "new.h"


static size_t MIN_RANK_SAMPLE_INTERVAL =
    2*BYTESIZE; // (!) THIS HAS TO BE A MULTIPLE OF BYTESIZE (!)

struct _csrsbitarray {
	byte_t *data;
	size_t len;
	size_t byte_size;
	size_t total_bit_count[2];
	size_t rank_samples_bit_interval;
	size_t rank_samples_byte_interval;
	size_t rank_samples_count;
	size_t bytes_per_pos;
	size_t bytes_per_byte_pos;
	byte_t *rank_samples;
	size_t sel_samples_bit_interval[2];
	size_t sel_samples_count[2];
	byte_t *byte_sel_samples[2];
	byte_t *byte_sel_samples_corr[2];
};


static void init_rank_tables(csrsbitarray *ba)
{
	size_t byte_pos, group, cumul_rank, next_group_byte_pos;
	ba->rank_samples_bit_interval = MAX( MIN_RANK_SAMPLE_INTERVAL,
	                                     ( ((size_t)(pow(log2(ba->len),2)
	                                             / BYTESIZE)) * BYTESIZE ) );
	ba->rank_samples_byte_interval = ba->rank_samples_bit_interval / BYTESIZE;
	ba->rank_samples_count = MAX( (size_t) DIVCEIL(ba->len,
	                              ba->rank_samples_bit_interval), 1 );
	ba->rank_samples = bytearr_new(ba->rank_samples_count*ba->bytes_per_pos);

	group = 0;
	byte_pos = 0;
	next_group_byte_pos=0;
	cumul_rank = 0;
	while (group < ba->rank_samples_count) {
		// read bytes greedily on a per max word basis
#if BYTEWORDSIZE==8
		while (byte_pos+8 < next_group_byte_pos) {
			cumul_rank += uint64_bitcount1(*((uint64_t *)(ba->data+byte_pos)));
			byte_pos += 8;
		}
		while (byte_pos+4 < next_group_byte_pos) {
			cumul_rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
			byte_pos += 4;
		}
		while (byte_pos+2 < next_group_byte_pos) {
			cumul_rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
			byte_pos += 2;
		}
#elif BYTEWORDSIZE==4
		while (byte_pos+4 < next_group_byte_pos) {
			cumul_rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
			byte_pos += 4;
		}
		while (byte_pos+2 < next_group_byte_pos) {
			cumul_rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
			byte_pos += 2;
		}
#endif
		while (byte_pos < next_group_byte_pos) {
			cumul_rank += byte_bitcount1(ba->data[byte_pos]);
			byte_pos++;
		}
		bytearr_write_size_t( ba->rank_samples, group*ba->bytes_per_pos,
		                      cumul_rank, ba->bytes_per_pos );
		next_group_byte_pos += ba->rank_samples_byte_interval;
		group++;
	}

	// finish up by counting the total number of 1s in the array
	if (byte_pos*BYTESIZE < ba->len) {
#if BYTEWORDSIZE==8
		while (byte_pos+8 < ba->byte_size-1) {
			cumul_rank += uint64_bitcount1(*((uint64_t *)(ba->data+byte_pos)));
			byte_pos += 8;
		}
		while (byte_pos+4 < ba->byte_size-1) {
			cumul_rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
			byte_pos += 4;
		}
		while (byte_pos+2 < ba->byte_size-1) {
			cumul_rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
			byte_pos += 2;
		}
#elif BYTEWORDSIZE==4
		while (byte_pos+4 < ba->byte_size-1) {
			cumul_rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
			byte_pos += 4;
		}
		while (byte_pos+2 < ba->byte_size-1) {
			cumul_rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
			byte_pos += 2;
		}
#endif
		while (byte_pos < ba->byte_size-1) {
			cumul_rank += byte_bitcount1(ba->data[byte_pos]);
			byte_pos++;
		}
		//count the last few bits, if any
		cumul_rank += byte_bitcount1(ba->data[byte_pos]&MSBMASK(((
		                                 ba->len-1)%BYTESIZE)+1));
	}
	ba->total_bit_count[1] = cumul_rank;
	ba->total_bit_count[0] = ba->len - cumul_rank;
}


static void init_select_tables(csrsbitarray *ba)
{
	size_t byte_pos, group, chunk_rank, cumul_rank, target_rank;

	ba->sel_samples_bit_interval[0] =
	    MAX( MIN_RANK_SAMPLE_INTERVAL,
	         (((size_t)(pow(log2(ba->total_bit_count[0]),2)/BYTESIZE))
	          * BYTESIZE) );
	ba->sel_samples_bit_interval[1] =
	    MAX( MIN_RANK_SAMPLE_INTERVAL,
	         (((size_t)(pow(log2(ba->total_bit_count[1]),2)/BYTESIZE))
	          * BYTESIZE) );
	ba->sel_samples_count[0] = MAX( DIVCEIL(ba->total_bit_count[0],
	                                        ba->sel_samples_bit_interval[0]),
	                                1 );
	ba->sel_samples_count[1] = MAX( DIVCEIL( ba->total_bit_count[1],
	                                ba->sel_samples_bit_interval[1]),
	                                1 );
	ba->byte_sel_samples[0] = bytearr_new( ba->sel_samples_count[0]
	                                       * ba->bytes_per_byte_pos );
	ba->byte_sel_samples[1] = bytearr_new( ba->sel_samples_count[1]
	                                       * ba->bytes_per_byte_pos );
	ba->byte_sel_samples_corr[0] = bytearr_new(ba->sel_samples_count[0]);
	ba->byte_sel_samples_corr[1] = bytearr_new(ba->sel_samples_count[1]);

	for (byte_t bit=0; bit<=1; bit++) {
		byte_pos = 0;
		group = 0;
		cumul_rank = 0;
		chunk_rank = 0;
		target_rank = 0;

		// creates at least one select sample for the 1st bit, setting its
		// position to N (byte) by default if it doesnt exist.
		// if it does, the correct value will be found below
		bytearr_write_size_t(ba->byte_sel_samples[bit], 0, ba->byte_size,
		                     ba->bytes_per_byte_pos);
		bytearr_write_size_t(ba->byte_sel_samples_corr[bit], 0, ba->len, 0);

		while (target_rank < ba->total_bit_count[bit]) {
			// read bytes greedily on a per max word basis
#if BYTEWORDSIZE==8
			chunk_rank = uint64_bitcount( *((uint64_t *)(ba->data+byte_pos)),
			                              bit );
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 8;
				chunk_rank = uint64_bitcount(*((uint64_t *)(ba->data+byte_pos)),
				                             bit );
			}
			chunk_rank = uint32_bitcount( *((uint32_t *)(ba->data+byte_pos)),
			                              bit );
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 4;
				chunk_rank = uint32_bitcount(*((uint32_t *)(ba->data+byte_pos)),
				                             bit);
			}
			chunk_rank = uint16_bitcount( *((uint16_t *)(ba->data+byte_pos)),
			                              bit );
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 2;
				chunk_rank = uint16_bitcount(*((uint16_t *)(ba->data+byte_pos)),
				                             bit);
			}
#elif BYTEWORDSIZE==4
			chunk_rank = uint32_bitcount( *((uint32_t *)(ba->data+byte_pos)),
			                              bit );
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 4;
				chunk_rank = uint32_bitcount(*((uint32_t *)(ba->data+byte_pos)),
				                             bit);
			}
			chunk_rank = uint16_bitcount( *((uint16_t *)(ba->data+byte_pos)),
			                              bit );
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 2;
				chunk_rank = uint16_bitcount(*((uint16_t *)(ba->data+byte_pos)),
				                             bit);
			}
#endif
			chunk_rank = byte_bitcount(*((byte_t *)(ba->data+byte_pos)), bit);
			while (cumul_rank+chunk_rank < target_rank) {
				cumul_rank += chunk_rank;
				byte_pos += 1;
				chunk_rank = byte_bitcount( *((byte_t *)(ba->data+byte_pos)),
				                            bit );
			}

			bytearr_write_size_t( ba->byte_sel_samples[bit],
			                      group*ba->bytes_per_byte_pos,
			                      byte_pos, ba->bytes_per_byte_pos );

			// add select samples corrections
			ba->byte_sel_samples_corr[bit][group] = (byte_t)(target_rank-cumul_rank);

			group++;
			target_rank += ba->sel_samples_bit_interval[bit];
		}
	}
}


csrsbitarray *csrsbitarr_new(byte_t *ba, size_t len)
{
	csrsbitarray *ret;
	ret = NEW(csrsbitarray);
	ret->data = ba;
	ret->len = len;
	ret->byte_size = DIVCEIL(ret->len, BYTESIZE);
	// use the minimum number of "bytes" per bit and byte position
	ret->bytes_per_pos = (size_t) DIVCEIL((size_t) ceil(log2(ret->len + 1)),
	                                      BYTESIZE);
	ret->bytes_per_byte_pos = (size_t) DIVCEIL((size_t) ceil(log2(
	                              ret->byte_size + 1)), BYTESIZE);

	init_rank_tables(ret);
	init_select_tables(ret);
	return ret;
}

void csrsbitarr_free(csrsbitarray *ba, bool free_data)
{
	if (ba == NULL) return;
	if (free_data) {
		FREE(ba->data);
	}
	FREE(ba->rank_samples);
	for (size_t b=0; b<2; b++) {
		FREE(ba->byte_sel_samples[b]);
		FREE(ba->byte_sel_samples_corr[b]);
	}
	FREE(ba);
}


const byte_t *csrsbitarr_data(csrsbitarray *ba)
{
	return ba->data;
}


size_t csrsbitarr_len(csrsbitarray *ba)
{
	return ba->len;
}


void csrsbitarr_print(FILE *stream, csrsbitarray *ba, size_t bytes_per_row)
{
	fprintf(stream, "csrsbitarray@%p {\n",(void *)ba);
	fprintf(stream, "->size = %zu\n",ba->len);
	fprintf(stream, "->data:\n");
	bitarr_print(stream, ba->data, ba->len, 4);
	for (unsigned int b=0; b<=1; b++) {
		fprintf(stream, "->total_bits_count[%u] = %zu\n",b,  ba->total_bit_count[b]);
	}
	fprintf(stream, "->rank_bit_sample_interval = %zu\n",
	        ba->rank_samples_bit_interval);
	//fprintf(stream, "->rank_byte_sample_interval = %zu\n", ba->rank_samples_byte_interval);
	fprintf(stream, "->rank_samples_count = %zu\n", ba->rank_samples_count);
	//fprintf(stream, "->bits_per_rank = %zu\n", ba->bits_per_pos);
	fprintf(stream, "->bytes_per_rank = %zu\n", ba->bytes_per_pos);
	fprintf(stream, "->rank_samples:\n");
	for (size_t i = 0; i  < ba->rank_samples_count; i++ ) {
		fprintf(stream,  "    rank_sample[%zu] = %zu\n", i,
		        bytearr_read_size_t( ba->rank_samples,i*ba->bytes_per_pos,
		                             ba->bytes_per_pos ) );
	}
	for (unsigned int b=0; b<=1; b++) {
		fprintf(stream,  "->select_samples_bit_interval[%u] = %zu\n", b,
		        ba->sel_samples_bit_interval[b] );
		fprintf(stream,  "->select_samples_count[%u] = %zu\n", b,
		        ba->sel_samples_count[b] );
		fprintf(stream, "->byte_select_samples[%u]:\n",b);
		for (size_t i = 0; i  < ba->sel_samples_count[b]; i++ ) {
			fprintf(stream, "    byte_select_sample[%u][%zu] = %zu\n", b, i,
			        bytearr_read_size_t( ba->byte_sel_samples[b],
			                             i*ba->bytes_per_byte_pos,
			                             ba->bytes_per_byte_pos));
		}
		fprintf(stream, "->select_samples_corrections[%u]:\n",b);
		for (size_t i = 0; i  < ba->sel_samples_count[b]; i++ ) {
			fprintf(stream, "    byte_select_sample_corr[%u][%zu] = %zu\n", b, i,
			        bytearr_read_size_t(ba->byte_sel_samples_corr[b], i, 1));
		}
	}
	//bytearr_print(ba->rank_samples, ba->rank_samples_count*ba->bytes_per_pos, 4);
	//fprintf(stream, "->select_samples[1]:\n");
	//bytearr_print(ba->select_samples[1], ba->select_samples_count[1]*ba->bytes_per_pos, 4);
	//fprintf(stream, "->byte_select_samples[1]:\n");
	//bytearr_print(ba->byte_select_samples[1], ba->select_samples_count[1]*ba->bytes_per_pos, 4);
	fprintf(stream, "}//end of csrsbitarray@%p\n",(void *)ba);
}

bool csrsbitarr_get(csrsbitarray *ba, size_t pos)
{
	return bitarr_get_bit(ba->data, pos);
}

size_t csrsbitarr_rank0(csrsbitarray *ba, size_t pos)
{
	if (pos >= ba->len) return ba->total_bit_count[0];

	return (pos-csrsbitarr_rank1(ba, pos));
}

size_t csrsbitarr_rank1(csrsbitarray *ba, size_t pos)
{
	size_t rank, group, sel_grp, byte_sel_smpl, byte_pos, last_byte;

	if (pos >= ba->len) return ba->total_bit_count[1];

	// go directly to the rank sample group
	group = pos / ba->rank_samples_bit_interval;
	rank = bytearr_read_size_t(ba->rank_samples, group*ba->bytes_per_pos,
	                           ba->bytes_per_pos);
	byte_pos = (group*ba->rank_samples_bit_interval) / BYTESIZE;
	last_byte = pos / BYTESIZE;

	// try to go to the last selection sample stop before pos
	sel_grp = rank/ba->sel_samples_bit_interval[1];
	while ( sel_grp < ba->sel_samples_count[1]-1 &&
	        bytearr_read_size_t( ba->byte_sel_samples[1],
	                             (sel_grp+1)*ba->bytes_per_byte_pos,
	                             ba->bytes_per_byte_pos ) * BYTESIZE < pos )
		sel_grp++;

	byte_sel_smpl = bytearr_read_size_t( ba->byte_sel_samples[1],
	                                     sel_grp*ba->bytes_per_byte_pos,
	                                     ba->bytes_per_byte_pos );
	if ( byte_pos < byte_sel_smpl
	        && byte_sel_smpl <= last_byte
	        && byte_sel_smpl < ba->byte_size ) {
		byte_pos = byte_sel_smpl;
		rank = (sel_grp*ba->sel_samples_bit_interval[1])
		       - ba->byte_sel_samples_corr[1][sel_grp];
	}

	//then complement partial result with local count
	// read bytes greedily on a per max word basis
#if BYTEWORDSIZE==8
	while (byte_pos+8 < last_byte) {
		rank += uint64_bitcount1(*((uint64_t *)(ba->data+byte_pos)));
		byte_pos += 8;
	}
	while (byte_pos+4 < last_byte) {
		rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
		byte_pos += 4;
	}
	while (byte_pos+2 < last_byte) {
		rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
		byte_pos += 2;
	}
#elif BYTEWORDSIZE==4
	while (byte_pos+4 < last_byte) {
		rank += uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos)));
		byte_pos += 4;
	}
	while (byte_pos+2 < last_byte) {
		rank += uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos)));
		byte_pos += 2;
	}
#endif
	while (byte_pos < last_byte) {
		rank += byte_bitcount1(ba->data[byte_pos]);
		byte_pos++;
	}
	//count the last few bits, if any
	rank += byte_bitcount1( ba->data[byte_pos] & MSBMASK((
	                            pos%BYTESIZE)));//////////////+1)  );
	return rank;
}


size_t csrsbitarr_rank(csrsbitarray *ba, size_t pos, bool bit)
{
	return bit ? csrsbitarr_rank1(ba, pos) : csrsbitarr_rank0(ba, pos);
}


size_t csrsbitarr_select0(csrsbitarray *ba, size_t rank)
{
	if (rank >= ba->total_bit_count[0]) return ba->len;

	size_t group, byte_pos, byte_sel, cumul_rank, chunk_rank,
	       last_byte;

	last_byte = ba->len / BYTESIZE;

	group = rank / ba->sel_samples_bit_interval[0];
	byte_pos = bytearr_read_size_t( ba->byte_sel_samples[0],
	                                group*ba->bytes_per_byte_pos,
	                                ba->bytes_per_byte_pos );

	if (byte_pos>last_byte) return ba->len;

	cumul_rank = group*ba->sel_samples_bit_interval[0]
	             - ba->byte_sel_samples_corr[0][group];

	/* *************************************************************
	 * CANT DO THIS WITHOUT RANK0 SAMPLES
	 *
	rank_group = cumul_rank / ba->rank_samples_bit_interval;
	while ( rank_group < ba->rank_samples_count - 1  &&
	        bytearr_read_size_t( ba->rank_samples,
	                           (rank_group+1)*ba->bytes_per_pos,
	                           ba->bytes_per_pos ) <= rank )
	    rank_group ++;

	if (byte_pos < rank_group*ba->rank_samples_byte_interval) {
	    byte_pos = rank_group * ba->rank_samples_byte_interval;
	    cumul_rank = bytearr_read_size_t( ba->rank_samples,
	                                    rank_group*ba->bytes_per_pos,
	                                    ba->bytes_per_pos );
	}
	****************************************************************/

#if BYTEWORDSIZE==8
	while ( byte_pos+8 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint64_bitcount0(*((uint64_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 8;
	}
	while ( byte_pos+4 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint32_bitcount0(*((uint32_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 4;
	}
	while ( byte_pos+2 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint16_bitcount0(*((uint16_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 2;
	}
#elif BYTEWORDSIZE==4
	while ( byte_pos+4 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint32_bitcount0(*((uint32_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 4;
	}
	while ( byte_pos+2 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint16_bitcount0(*((uint16_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 2;
	}
#endif
	while ( byte_pos < last_byte &&
	        cumul_rank
	        + (chunk_rank=byte_bitcount0(*((byte_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 1;
	}
	byte_sel = byte_select0(ba->data[byte_pos], rank-cumul_rank);

	return MIN(ba->len, byte_pos*BYTESIZE+byte_sel);
}


size_t csrsbitarr_select1(csrsbitarray *ba, size_t rank)
{
	if (rank >= ba->total_bit_count[1]) return ba->len;

	size_t group, byte_pos, rank_group, byte_sel, cumul_rank, chunk_rank,
	       last_byte;

	last_byte = ba->len / BYTESIZE;

	group = rank / ba->sel_samples_bit_interval[1];
	byte_pos = bytearr_read_size_t( ba->byte_sel_samples[1],
	                                group*ba->bytes_per_byte_pos,
	                                ba->bytes_per_byte_pos );

	if (byte_pos>last_byte) return ba->len;

	cumul_rank = group*ba->sel_samples_bit_interval[1]
	             - ba->byte_sel_samples_corr[1][group];


	rank_group = cumul_rank / ba->rank_samples_bit_interval;
	while ( rank_group < ba->rank_samples_count - 1  &&
	        bytearr_read_size_t( ba->rank_samples,
	                             (rank_group+1)*ba->bytes_per_pos,
	                             ba->bytes_per_pos ) <= rank )
		rank_group ++;

	if (byte_pos < rank_group*ba->rank_samples_byte_interval) {
		byte_pos = rank_group * ba->rank_samples_byte_interval;
		cumul_rank = bytearr_read_size_t( ba->rank_samples,
		                                  rank_group*ba->bytes_per_pos,
		                                  ba->bytes_per_pos );
	}

#if BYTEWORDSIZE==8
	while ( byte_pos+8 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint64_bitcount1(*((uint64_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 8;
	}
	while ( byte_pos+4 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 4;
	}
	while ( byte_pos+2 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 2;
	}
#elif BYTEWORDSIZE==4
	while ( byte_pos+4 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint32_bitcount1(*((uint32_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 4;
	}
	while ( byte_pos+2 < last_byte &&
	        cumul_rank
	        + (chunk_rank=uint16_bitcount1(*((uint16_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 2;
	}
#endif
	while ( byte_pos < last_byte &&
	        cumul_rank
	        + (chunk_rank=byte_bitcount1(*((byte_t *)(ba->data+byte_pos))))
	        <= rank ) {
		cumul_rank += chunk_rank;
		byte_pos += 1;
	}
	byte_sel = byte_select1(ba->data[byte_pos], rank-cumul_rank);

	return MIN(ba->len, byte_pos*BYTESIZE+byte_sel);
}


size_t csrsbitarr_select(csrsbitarray *ba, size_t rank, bool bit)
{

	return bit ? csrsbitarr_select1(ba, rank) : csrsbitarr_select0(ba, rank);
}



size_t csrsbitarr_pred0(csrsbitarray *ba, size_t pos)
{
	size_t rank = csrsbitarr_rank0(ba, pos);
	return (rank>0) ? csrsbitarr_select0(ba, rank-1) : ba->len;
}


size_t csrsbitarr_pred1(csrsbitarray *ba, size_t pos)
{
	size_t rank = csrsbitarr_rank1(ba, pos);
	return (rank>0) ? csrsbitarr_select1(ba, rank-1) : ba->len;
}


size_t csrsbitarr_pred(csrsbitarray *ba, size_t pos, bool bit)
{
	return bit ? csrsbitarr_pred1(ba, pos) : csrsbitarr_pred0(ba, pos);
}



size_t csrsbitarr_succ0(csrsbitarray *ba, size_t pos)
{
	if (pos>=ba->len) return ba->len;
	if (csrsbitarr_get(ba, pos)==0)
		return csrsbitarr_select0(ba, csrsbitarr_rank0(ba, pos)+1);
	else
		return csrsbitarr_select0(ba, csrsbitarr_rank0(ba, pos));
}


size_t csrsbitarr_succ1(csrsbitarray *ba, size_t pos)
{
	if (pos>=ba->len) return ba->len;
	if (csrsbitarr_get(ba, pos)==1)
		return csrsbitarr_select1(ba, csrsbitarr_rank1(ba, pos)+1);
	else
		return csrsbitarr_select1(ba, csrsbitarr_rank1(ba, pos));
}


size_t csrsbitarr_succ(csrsbitarray *ba, size_t pos, bool bit)
{
	return bit ? csrsbitarr_succ1(ba, pos) : csrsbitarr_succ0(ba, pos);
}
