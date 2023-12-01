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

#include "bitbyte.h"
#include "coretype.h"
#include "errlog.h"
#include "avlmap.h"
#include "hash.h"
#include "mathutil.h"
#include "new.h"
#include "vebset.h"


typedef uint16_t vebleaf16_t;

#define MASK1(bit) ((uint16_t)(1<<(bit)))
#define MASK0(bit) (~(MASK1(bit)))

vebleaf16_t *vebleaf16_new() {
	vebleaf16_t *ret = NEW(vebleaf16_t);
	*ret = 0;
	return ret;
}


bool vebleaf16_add(vebleaf16_t *leaf, uint32_t x)
{
	assert(x < 16);
	if (*leaf & MASK1(x)) { // is bit x set?
		return false;
	}
	*leaf |= (MASK1(x));	
	return true;
}


bool vebleaf16_del(vebleaf16_t *leaf, uint32_t x)
{
	assert(x < 16);
	if (*leaf & MASK1(x)) { // is bit x set?
		*leaf &= (MASK0(x));
		return true;
	}
	return false;
}


bool vebleaf16_contains(vebleaf16_t *leaf, uint32_t x)
{
	assert(x < 16);
	return (bool)(*leaf & MASK1(x));	
}


bool vebleaf16_empty(vebleaf16_t *leaf) 
{
	return (*leaf) == 0;
}


int64_t vebleaf16_min(vebleaf16_t *leaf) 
{
	return uint16_lobit(*leaf);
}


int64_t vebleaf16_max(vebleaf16_t *leaf) 
{
	int64_t ret = uint16_hibit(*leaf);
	return (ret < 16) ? ret : -1;
}


size_t vebleaf16_size(vebleaf16_t *leaf) 
{
	return uint16_bitcount1(*leaf);
}


int64_t vebleaf16_pred(vebleaf16_t *leaf, uint32_t x)
{
	assert (x < 16);
	int64_t ret = uint16_hibit(*leaf & ((uint16_t)0xFFFF >> (16 - x)));
	return (ret < 16) ? ret : -1;
}


int64_t vebleaf16_succ(vebleaf16_t *leaf, uint32_t x)
{
	assert (x < 16);
	uint ret = uint16_lobit(*leaf & ((uint16_t)0xFFFF << (x+1)));
	return ret;
}


#define UNIV(nbits) ((int64_t)1 << nbits)

#define SQRTUNIV(nbits) ((int64_t)1 << (nbits/2))

#define HIGH(i, nbits) (i / SQRTUNIV(nbits))

#define LOW(i, nbits) (i % SQRTUNIV(nbits))

#define INDEX(h, l,  nbits) ( (h * SQRTUNIV(nbits)) + l)

#define CLUSTER(index) avlmap_get_rawptr(self->clusters, &index)


typedef struct _vebnode {
	int64_t min;
	int64_t max;
	void *summary;
	avlmap *clusters;
} vebnode;


void *vebnode_new_sized(uint nbits)
{
	if (nbits==4) {
		return vebleaf16_new();
	}
	vebnode *ret = NEW(vebnode);
	ret->min = UNIV(nbits);
	ret->max = -1;
	ret->summary = NULL; // initialize summary and clusters on demand
	ret->clusters = NULL;
	return ret;
}


void vebnode_free(void *ptr, uint nbits)
{
	if (!ptr) {
		return;
	}
	if (nbits == 4) {
		FREE(ptr);
		return;
	}
	vebnode *self = (vebnode *)ptr;
	if (self->summary) {
		vebnode_free(self->summary, nbits/2);
	}
	if (self->clusters) {
		avlmap_iter *it = avlmap_get_iter(self->clusters, IN_ORDER);
		FOREACH_IN_ITER(entry, avlmap_entry, avlmap_iter_as_iter(it)) {
			vebnode_free(*((void**)(entry->val)), nbits/2);
		}
		avlmap_iter_free(it);
		DESTROY_FLAT(self->clusters, avlmap);
	}
	FREE(self);
}



bool vebnode_empty(void *self, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_empty(self);
	}
	return ((vebnode*)self)->max < 0;
}


int64_t vebnode_min(void *self, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_min(self);
	}
	return ((vebnode*)self)->min;
}


int64_t vebnode_max(void *self, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_max(self);
	}
	return ((vebnode*)self)->max;
}


bool vebnode_contains(void *root, uint32_t x, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_contains(root, x);
	}
	vebnode *self = (vebnode *)root;
	if (x < self->min || x > self->max) {
		return false;
	}
	if (x == self->min || x == self->max) {
		return true;
	}
	uint32_t high = HIGH(x, nbits);
	if (avlmap_contains(self->clusters, &high)) {
		void *cluster = CLUSTER(high);
		uint32_t low = LOW(x, nbits);
		return vebnode_contains(cluster, low, nbits/2);
	}
	else {
		return false;
	}
}


bool vebnode_add(void *root, uint32_t x, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_add(root, x);
	}
	vebnode *self = (vebnode *)root;
	if (x >= UNIV(nbits) || x == self->min || x == self->max) {
		return false;
	}
	bool ret = false;
	if (vebnode_empty(self, nbits)) {
		//DEBUG("vebnode at level %d bits is empty\n", self->nbits);
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
		self->summary = vebnode_new_sized(nbits/2);
		self->clusters = avlmap_new(sizeof(uint32_t), sizeof(void *), cmp_uint32_t);
	}
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);
	void *cluster = NULL;
	if (!avlmap_contains(self->clusters, &high)) {
		cluster = vebnode_new_sized(nbits/2);
		avlmap_ins_rawptr(self->clusters, &high, cluster);
	} else {
		cluster = CLUSTER(high);
	}
	if (vebnode_empty(cluster, nbits/2)) {
		vebnode_add(self->summary, high, nbits/2);
	}
	ret = vebnode_add(cluster, low, nbits/2);
	return ret;
}


bool vebnode_del(void *root, uint32_t x, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_del(root, x);
	}
	vebnode *self = (vebnode *)root;
	if (x >= UNIV(nbits) || vebnode_empty(self, nbits)) {
		return false;
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
		assert(!vebnode_empty(self->summary, nbits/2));
		uint32_t high = (uint32_t) vebnode_min(
		                    self->summary, nbits/2); // first non-empty-cluster
		void *cluster = CLUSTER(high);
		assert(!vebnode_empty(cluster, nbits/2));
		uint32_t low = (uint32_t)vebnode_min(cluster, nbits/2);
		self->min = INDEX(high, low, nbits);
		x = self->min; // put the min in x to be removed recursively
	}
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);
	if (!avlmap_contains(self->clusters, &high)) {
		return false;
	}
	void *cluster = CLUSTER(high);
	deleted = vebnode_del(cluster, low, nbits/2);
	if (deleted && vebnode_empty(cluster, nbits/2)) {
		vebnode_del(self->summary, high, nbits/2);
	}
	if (x == self->max) {
		if (vebnode_empty(self->summary, nbits/2)) { // no element left other than the min
			assert (self->min < UNIV(nbits));
			self->max = self->min;
		}
		else {   // set "previous" second to last element as max.
			// (!) now it is physically the last since x already recursively removed
			uint32_t high = vebnode_max(self->summary, nbits/2);
			void *cluster = CLUSTER(high);
			uint32_t low = vebnode_max(cluster, nbits/2);
			self->max = INDEX(high, low, nbits);
		}
	}
	return deleted;
}


int64_t vebnode_succ(void *root, uint32_t x, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_succ(root, x);
	}
	vebnode *self = (vebnode *)root;
	if (x >= self->max) {
		return UNIV(nbits);
	}
	if (x < self->min) {
		return self->min;
	}
	if (vebnode_empty(self, nbits)) {
		return UNIV(nbits);
	}
	assert(self->summary);
	assert(self->clusters);
	uint32_t high = HIGH(x, nbits);
	uint32_t low = LOW(x, nbits);

	vebnode *cluster = NULL;
	if (avlmap_contains(self->clusters, &high)) {
		cluster = CLUSTER(high);
	}
	if (cluster && low < vebnode_max(cluster, nbits/2)) {
		// sucessor in the same cluster as x
		low = vebnode_succ(cluster, low, nbits/2);
	}
	else {
		// sucessor not in the same cluster as x
		high = vebnode_succ(self->summary, high, nbits/2);
		if (high < UNIV(nbits/2)) {
			assert(avlmap_contains(self->clusters, &high));
			cluster = CLUSTER(high);
			low = vebnode_min(cluster, nbits/2);
		}
		else {
			return UNIV(nbits);
		}
	}
	return INDEX(high, low, nbits);
}


int64_t vebnode_pred(void *root, uint32_t x, uint nbits)
{
	assert (nbits >= 4);
	if (nbits == 4) {
		return vebleaf16_pred(root, x);
	}
	vebnode *self = (vebnode *)root;
	//DEBUG("looking for predecessor of %u at @%p nbits = %u\n", x, self, nbits);
	if (x <= self->min) {
		//DEBUG("x=%u <= min=%ld. no pred. return -1\n", x, self->min);
		return -1;
	}
	if (x > self->max) {
		//DEBUG("x=%u > max=%ld. return max\n", x, self->max);
		return self->max;
	}
	if (vebnode_empty(self, nbits)) {
		return -1;
	}
	assert(self->summary);
	assert(self->clusters);
	int64_t high = HIGH(x, nbits);
	int64_t low = LOW(x, nbits);

	void *cluster = NULL;
	if (avlmap_contains(self->clusters, &high)) {
		cluster = CLUSTER(high);
	}
	if (cluster &&  vebnode_min(cluster, nbits/2) < low) {
		assert(vebnode_min(cluster, nbits/2) < UNIV(nbits/2));
		// predecessor in the same cluster as x
		//DEBUG("pred in same cluster #%u as x. Looking therein\n", high);
		low = vebnode_pred(cluster, low, nbits/2);
		assert(vebnode_min(cluster, nbits/2) <= low);
		//DEBUG("found pred in in cluster #%u at low = %ld\n", high, low);
	}
	else {
		// predecessor not in the same cluster as x
		//DEBUG("pred not in same cluster #%u as x=%u. looking for its pred in summary\n", high, x);
		high = vebnode_pred(self->summary, high, nbits/2);
		//DEBUG("summary indicated pred is in cluster #%ld\n", high);
		if (high >= 0) {
			assert(avlmap_contains(self->clusters, &high));
			cluster = CLUSTER(high);
			low = vebnode_max(cluster, nbits/2);
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
	vebnode *tree;
};


vebset *vebset_new()
{
	vebset *ret = NEW(vebset);
	ret->size = 0;
	ret->nbits = 32;
	ret->tree = vebnode_new_sized(ret->nbits);
	return ret;
}


void vebset_finalise(void *ptr, const finaliser *fnr) 
{
	vebset *self = (vebset *)ptr;
	vebnode_free(self->tree, self->nbits);
	FREE(ptr);
}


void vebset_free(vebset *self)
{
	vebnode_free(self->tree, self->nbits);
	FREE(self);
}


size_t vebset_size(vebset *self)
{
	return self->size;
}


bool vebset_contains(vebset *self, uint32_t x)
{
	return vebnode_contains(self->tree, x, self->nbits);
}


bool vebset_add(vebset *self, uint32_t x)
{
	bool ret = vebnode_add(self->tree, x, self->nbits);
	self->size += ret;
	return ret;
}


bool vebset_del(vebset *self, uint32_t x)
{
	bool ret = vebnode_del(self->tree, x, self->nbits);
	self->size -= ret;
	return ret;
}


int64_t vebset_succ(vebset *self, uint32_t x)
{
	return vebnode_succ(self->tree, x, self->nbits);
}


int64_t vebset_pred(vebset *self, uint32_t x)
{
	return vebnode_pred(self->tree, x, self->nbits);
}


int64_t vebset_min(vebset *self)
{
	return vebnode_min(self->tree, self->nbits);
}


int64_t vebset_max(vebset *self)
{
	return vebnode_max(self->tree, self->nbits);
}

