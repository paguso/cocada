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

#include <assert.h>

#include "coretype.h"
#include "errlog.h"
#include "hashmap.h"
#include "hash.h"
#include "mathutil.h"
#include "new.h"
#include "vebset.h"


#define UNIV(nbits) ((int64_t)1 << nbits)

#define SQRTUNIV(nbits) ((int64_t)1 << (nbits/2))

#define HIGH(i, nbits) (i / SQRTUNIV(nbits))

#define LOW(i, nbits) (i % SQRTUNIV(nbits))

#define INDEX(h, l,  nbits) ( (h * SQRTUNIV(nbits)) + l)

#define CLUSTER(index) ((vebtree*) hashmap_get_rawptr(self->clusters, &index))



typedef struct _vebtree {
	int64_t min;
	int64_t max;
	struct _vebtree *summary;
	hashmap *clusters;
} vebtree;


vebtree *vebtree_new_sized(uint nbits)
{
	if (nbits != 32 && nbits != 16 && nbits != 8 && nbits != 4 && nbits != 2
	        && nbits != 1) {
		WARN("vebtree invalid number of bits %u. Must be 64, 32, 16, 8, 4, 2 or 1.",
		     nbits);
		return NULL;
	}
	vebtree *ret = NEW(vebtree);
	ret->min = UNIV(nbits);
	ret->max = -1;
	if (nbits == 1) {
		ret->summary = NULL;
		ret->clusters = NULL;
	}
	else {
		ret->summary = NULL;// vebtree_new_sized(nbits / 2);
		ret->clusters =
		    NULL;//hashmap_new(sizeof(size_t), sizeof(vebtree *), ident_hash_size_t, eq_size_t);
	}
	return ret;
}


void vebtree_finalise(void *self, const finaliser *fnr)
{
	if (!self) {
		return;
	}
	vebtree *vset = (vebtree *)self;
	if (vset->summary) {
		DESTROY_FLAT(vset->summary, vebtree);
	}
	if (vset->clusters) {
		DESTROY(vset->clusters,
		        finaliser_cons(
		            finaliser_cons(FNR(hashmap),
		                           finaliser_new_empty()),
		            finaliser_new_ptr_to_obj(FNR(vebtree))));
	}
}


void vebtree_free(vebtree *self)
{
	DESTROY_FLAT(self, vebtree);
}


bool vebtree_empty(vebtree *self)
{
	return self->max < 0;
}


int64_t vebtree_min(vebtree *self)
{
	return self->min;
}


int64_t vebtree_max(vebtree *self)
{
	return self->max;
}


bool vebtree_contains(vebtree *self, uint32_t x, uint nbits)
{
	if (x < self->min || x > self->max) {
		return false;
	}
	if (x == self->min || x == self->max) {
		return true;
	}
	uint32_t high = HIGH(x, nbits);
	if (hashmap_contains(self->clusters, &high)) {
		vebtree *cluster = (vebtree *)hashmap_get_rawptr(self->clusters, &high);
		uint32_t low = LOW(x, nbits);
		return vebtree_contains(cluster, low, nbits/2);
	}
	else {
		return false;
	}
}


bool vebtree_add(vebtree *self, uint32_t x, uint nbits)
{
	//DEBUG("Adding %u at level %u bits\n", x, self->nbits);
	if (x >= UNIV(nbits) || x == self->min || x == self->max) {
		return false;
	}
	bool ret = false;
	if (nbits == 1) {
		if (x < self->min) {
			self->min = x;
			ret = true;
		}
		if (x > self->max) {
			self->max = x;
			ret = true;
		}
		return ret;
	}
	if (vebtree_empty(self)) {
		//DEBUG("vebtree at level %d bits is empty\n", self->nbits);
		self->min = self->max = x;
		return true;
	}
	if (x < self->min) {
		uint32_t swp = self->min;
		self->min = x;
		x = swp;
		ret = true;
	}
	if (x > self->max) {
		self->max = x;
		ret = true;
	}
	// will insert x recursively. summary and cluster needed
	if (!self->summary) {
		self->summary = vebtree_new_sized(nbits/2);
		self->clusters = hashmap_new(sizeof(uint32_t), sizeof(vebtree *),
		                             ident_hash_uint32_t, eq_uint32_t);
	}
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);
	vebtree *cluster = NULL;
	if (!hashmap_contains(self->clusters, &high)) {
		cluster = vebtree_new_sized(nbits/2);
		//DEBUG("Creating new cluster #%u @%p at level %u bits (parent=%p).\n", high, cluster, self->nbits, self);
		hashmap_ins_rawptr(self->clusters, &high, cluster);
	}
	else {
		cluster = (vebtree *) hashmap_get_rawptr(self->clusters, &high);
		//DEBUG("Retrieving cluster #%u @%p at level %u bits (parent = %p).\n", high, cluster, self->nbits, self);
	}
	if (vebtree_empty(cluster)) {
		//DEBUG("Adding cluster #%u to summary\n", high);
		vebtree_add(self->summary, high, nbits/2);
	}
	//DEBUG("Adding %u recursively to cluster #%u\n", low, high);
	ret = vebtree_add(cluster, low, nbits/2);
	return ret;
}


bool vebtree_del(vebtree *self, uint32_t x, uint nbits)
{
	if (x >= UNIV(nbits) || vebtree_empty(self)) {
		return false;
	}
	if (nbits == 1) {
		bool deleted = false;
		if (x == self->min) {
			self->min = (self->max != x) ? self->max : 2;
			deleted = true;
		}
		if (x == self->max) {
			self->max = (self->min != 2) ? self->min : -1;
			deleted = true;
		}
		return deleted;
	}
	bool deleted = false;
	if (x == self->min) {
		deleted = true;
		if (x == self->max) { // x = min = max is the only element left in the set
			self->min = UNIV(nbits);
			self->max = -1;
			return true;      // not recursively stored: go home
		}
		// there are elements other than the min
		// pull the second smallest value to substitute if for the min
		assert(!vebtree_empty(self->summary));
		uint32_t high = (uint32_t) vebtree_min(
		                    self->summary); // first non-empty-cluster
		vebtree *cluster = CLUSTER(high);
		assert(!vebtree_empty(cluster));
		uint32_t low = (uint32_t)vebtree_min(cluster);
		self->min = INDEX(high, low, nbits);
		x = self->min; // put the min in x to be removed recursively
	}
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);
	if (!hashmap_contains(self->clusters, &high)) {
		return false;
	}
	vebtree *cluster = CLUSTER(high);
	deleted = vebtree_del(cluster, low, nbits/2);
	if (deleted && vebtree_empty(cluster)) {
		vebtree_del(self->summary, high, nbits/2);
	}
	if (x == self->max) {
		if (vebtree_empty(self->summary)) { // no element left other than the min
			assert (self->min < UNIV(nbits));
			self->max = self->min;
		}
		else {   // set "previous" second to last element as max.
			// (!) now it is physically the last since x already recursively removed
			uint32_t high = vebtree_max(self->summary);
			vebtree *cluster = CLUSTER(high);
			uint32_t low = vebtree_max(cluster);
			self->max = INDEX(high, low, nbits);
		}
	}
	return deleted;
}


int64_t vebtree_succ(vebtree *self, uint32_t x, uint nbits)
{
	if (x >= self->max) {
		return UNIV(nbits);
	}
	if (x < self->min) {
		return self->min;
	}
	if (nbits == 1) {
		if (x == 0 && self->max == 1) {
			return 1;
		}
		else {
			return 2;
		}
	}
	if (vebtree_empty(self)) {
		return UNIV(nbits);
	}
	assert(self->summary);
	assert(self->clusters);
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);

	vebtree *cluster = NULL;
	if (hashmap_contains(self->clusters, &high)) {
		cluster = (vebtree *) hashmap_get_rawptr(self->clusters, &high);
	}
	if (cluster && low < vebtree_max(cluster)) {
		// sucessor in the same cluster as x
		low = vebtree_succ(cluster, low, nbits/2);
	}
	else {
		// sucessor not in the same cluster as x
		high = vebtree_succ(self->summary, high, nbits/2);
		if (high < UNIV(nbits/2)) {
			assert(hashmap_contains(self->clusters, &high));
			cluster = (vebtree *) hashmap_get_rawptr(self->clusters, &high);
			low = vebtree_min(cluster);
		}
		else {
			return UNIV(nbits);
		}
	}
	return INDEX(high, low, nbits);
}


int64_t vebtree_pred(vebtree *self, uint32_t x, uint nbits)
{
	//DEBUG("looking for predecessor of %u at @%p nbits = %u\n", x, self, self->nbits);
	if (x <= self->min) {
		//DEBUG("x=%u <= min=%ld. no pred. return -1\n", x, self->min);
		return -1;
	}
	if (x > self->max) {
		//DEBUG("x=%u > max=%ld. return max\n", x, self->max);
		return self->max;
	}
	if (nbits == 1) {
		if (x == 1 && self->min == 0) {
			//DEBUG("x=1 and min=0. return 0\n");
			return 0;
		}
		else {
			//DEBUG("x=%ld and min=%ld. no pred. return -1\n", x, self->min);
			return -1;
		}
	}
	if (vebtree_empty(self)) {
		return -1;
	}
	assert(self->summary);
	assert(self->clusters);
	int64_t high = HIGH(x, nbits);
	int64_t low = LOW(x, nbits);

	vebtree *cluster = NULL;
	if (hashmap_contains(self->clusters, &high)) {
		cluster = (vebtree *) hashmap_get_rawptr(self->clusters, &high);
	}
	if (cluster &&  vebtree_min(cluster) < low) {
		assert(vebtree_min(cluster) < UNIV(nbits));
		// predecessor in the same cluster as x
		//DEBUG("pred in same cluster #%u as x. Looking therein\n", high);
		low = vebtree_pred(cluster, low, nbits/2);
		assert(vebtree_min(cluster) <= low);
		//DEBUG("found pred in in cluster #%u at low = %ld\n", high, low);
	}
	else {
		// predecessor not in the same cluster as x
		//DEBUG("pred not in same cluster #%u as x=%u. looking for its pred in summary\n", high, x);
		high = vebtree_pred(self->summary, high, nbits/2);
		//DEBUG("summary indicated pred is in cluster #%ld\n", high);
		if (high >= 0) {
			assert(hashmap_contains(self->clusters, &high));
			cluster = (vebtree *) hashmap_get_rawptr(self->clusters, &high);
			low = vebtree_max(cluster);
			assert (low >= 0);
		}
		else {
			if (self->min < UNIV(nbits)) {
				assert (self->min < x);
				//DEBUG("This should mean no pred, but this vEB has a min=%ld < %u. Therefore returning %ld\n", self->min, x, self->min);
				return self->min;
			}
			else {
				return -1;
			}
		}
	}
	int64_t index =  INDEX(high, low, nbits);
	return index;
}



struct _vebset {
	size_t size;
	uint nbits;
	vebtree *tree;
};


vebset *vebset_new()
{
	vebset *ret = NEW(vebset);
	ret->size = 0;
	ret->nbits = 32;
	ret->tree = vebtree_new_sized(ret->nbits);
	return ret;
}


void vebset_free(vebset *self)
{
	vebtree_free(self->tree);
	FREE(self);
}


size_t vebset_size(vebset *self)
{
	return self->size;
}


bool vebset_contains(vebset *self, uint32_t x)
{
	return vebtree_contains(self->tree, x, self->nbits);
}


bool vebset_add(vebset *self, uint32_t x)
{
	bool ret = vebtree_add(self->tree, x, self->nbits);
	self->size += ret;
	return ret;
}


bool vebset_del(vebset *self, uint32_t x)
{
	bool ret = vebtree_del(self->tree, x, self->nbits);
	self->size -= ret;
	return ret;
}


int64_t vebset_succ(vebset *self, uint32_t x)
{
	return vebtree_succ(self->tree, x, self->nbits);
}


int64_t vebset_pred(vebset *self, uint32_t x)
{
	return vebtree_pred(self->tree, x, self->nbits);
}


int64_t vebset_min(vebset *self)
{
	return vebtree_min(self->tree);
}


int64_t vebset_max(vebset *self)
{
	return vebtree_max(self->tree);
}

