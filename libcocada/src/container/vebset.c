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

#define CLUSTER(index) ((vebset*) hashmap_get_rawptr(self->clusters, &index))

struct _vebset {
    uint nbits;
    int64_t min;
    int64_t max;
    struct _vebset *summary;
    hashmap *clusters;    
};


vebset *vebset_new_sized(uint nbits)
{
    if (nbits != 32 && nbits != 16 && nbits != 8 && nbits != 4 && nbits != 2 && nbits != 1) {
        WARN("vebset invalid number of bits %u. Must be 64, 32, 16, 8, 4, 2 or 1.", nbits);
        return NULL;        
    }
    vebset *ret = NEW(vebset);
    ret->nbits = nbits;
    ret->min = UNIV(nbits);
    ret->max = -1;
    if (ret->nbits == 1) {
        ret->summary = NULL;
        ret->clusters = NULL;
    } else {
        ret->summary = NULL;// vebset_new_sized(nbits / 2);
        ret->clusters = NULL;//hashmap_new(sizeof(size_t), sizeof(vebset *), ident_hash_size_t, eq_size_t);
    }
    return ret; 
}


vebset *vebset_new()
{
    return vebset_new_sized(32);
}


void vebset_finalise(void *self, const finaliser *fnr) 
{
    if (!self) {
        return;
    }
    vebset *vset = (vebset *)self;
    if (vset->summary) {
        DESTROY_FLAT(vset->summary, vebset);        
    }
    if (vset->clusters) {
        DESTROY(vset->clusters, 
            finaliser_cons(
                finaliser_cons(FNR(hashmap),
                    finaliser_new_empty()), 
                finaliser_new_ptr_to_obj(FNR(vebset))));
    }
}


void vebset_free(vebset *self)
{
    DESTROY_FLAT(self, vebset);
}


int64_t vebset_univ(vebset *self) {
    return UNIV(self->nbits);
}


bool vebset_empty(vebset *self) {
    return self->min == UNIV(self->nbits);
}

bool vebset_contains(vebset *self, uint32_t x) 
{
    if (x < self->min || x > self->max) {
        return false;
    }
    if (x == self->min || x == self->max) {
        return true;
    }
    uint32_t high = HIGH(x, self->nbits);
    if (hashmap_contains(self->clusters, &high)) {
        vebset *cluster = (vebset*)hashmap_get_rawptr(self->clusters, &high);
        uint32_t low = LOW(x, self->nbits);
        return vebset_contains(cluster, low);
    } else {
        return false;
    }
}


bool vebset_add(vebset *self, uint32_t x) 
{
    //DEBUG("Adding %u at level %u bits\n", x, self->nbits);
    if (x >= UNIV(self->nbits) || x == self->min || x == self->max) {
        return false;
    }
    bool ret = false;
    if (self->nbits == 1) {
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
    if (vebset_empty(self)) {
        //DEBUG("vebset at level %d bits is empty\n", self->nbits);
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
        self->summary = vebset_new_sized(self->nbits/2);
        self->clusters = hashmap_new(sizeof(uint32_t), sizeof(vebset *), ident_hash_uint32_t, eq_uint32_t);
    }
    uint32_t high = HIGH(x, self->nbits);
    uint32_t low = LOW(x, self->nbits);
    vebset *cluster = NULL;
    if (!hashmap_contains(self->clusters, &high)) {
        cluster = vebset_new_sized(self->nbits/2);
        //DEBUG("Creating new cluster #%u @%p at level %u bits (parent=%p).\n", high, cluster, self->nbits, self);
        hashmap_ins_rawptr(self->clusters, &high, cluster);
    } else {
        cluster = (vebset*) hashmap_get_rawptr(self->clusters, &high);
        //DEBUG("Retrieving cluster #%u @%p at level %u bits (parent = %p).\n", high, cluster, self->nbits, self);
    }
    if (vebset_empty(cluster)) {
        //DEBUG("Adding cluster #%u to summary\n", high);
        vebset_add(self->summary, high);
    }
    //DEBUG("Adding %u recursively to cluster #%u\n", low, high);
    ret = vebset_add(cluster, low);
    return ret;
}


bool vebset_del(vebset *self, uint32_t x)
{
    if (x >= UNIV(self->nbits) || vebset_empty(self)) {
        return false;
    }
    if (self->nbits == 1) {
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
            self->min = UNIV(self->nbits);
            self->max = -1;
            return true;      // not recursively stored: go home
        }
        // there are elements other than the min
        // pull the second smallest value to substitute if for the min
        assert(!vebset_empty(self->summary));
        uint32_t high = (uint32_t) vebset_min(self->summary); // first non-empty-cluster
        vebset *cluster = CLUSTER(high);
        assert(!vebset_empty(cluster));
        uint32_t low = (uint32_t)vebset_min(cluster);
        self->min = INDEX(high, low, self->nbits);
        x = self->min; // put the min in x to be removed recursively
    }
    uint32_t high = HIGH(x, self->nbits);
    uint32_t low = LOW(x, self->nbits);
    if (!hashmap_contains(self->clusters, &high)) {
        return false;
    }
    vebset *cluster = CLUSTER(high);
    deleted = vebset_del(cluster, low);
    if (deleted && vebset_empty(cluster)) {
        vebset_del(self->summary, high);
    }
    if (x == self->max) {
        if (vebset_empty(self->summary)) { // no element left other than the min
            assert (self->min < UNIV(self->nbits));
            self->max = self->min;
        } else { // set "previous" second to last element as max. 
                 // (!) now it is physically the last since x already recursively removed
            uint32_t high = vebset_max(self->summary);
            vebset *cluster = CLUSTER(high);
            uint32_t low = vebset_max(cluster);
            self->max = INDEX(high, low, self->nbits);
        }
    }
    return deleted;
}





int64_t vebset_min(vebset *self) {
    return self->min;
} 


int64_t vebset_max(vebset *self) {
    return self->max;
}

int64_t vebset_succ(vebset *self, uint32_t x)
{
    if (x >= self->max) {
        return UNIV(self->nbits);
    }
    if (x < self->min) {
        return self->min;
    }
    if (self->nbits == 1) {
        if (x == 0 && self->max == 1) {
            return 1;
        } else {
            return 2;
        }
    }
    if (vebset_empty(self)) {
        return UNIV(self->nbits);
    }
    assert(self->summary);
    assert(self->clusters);
    uint32_t high = HIGH(x, self->nbits);
    uint32_t low = LOW(x, self->nbits);

    vebset *cluster = NULL;
    if (hashmap_contains(self->clusters, &high)) {
        cluster = (vebset*) hashmap_get_rawptr(self->clusters, &high);
    }
    if (cluster && low < vebset_max(cluster)) {
        // sucessor in the same cluster as x
        low = vebset_succ(cluster, low);
    } else {
        // sucessor not in the same cluster as x
        high = vebset_succ(self->summary, high);
        if (high < UNIV(self->summary->nbits)) {
            assert(hashmap_contains(self->clusters, &high));
            cluster = (vebset*) hashmap_get_rawptr(self->clusters, &high);
            low = vebset_min(cluster);
        } else {
            return UNIV(self->nbits);
        }
    } 
    return INDEX(high, low, self->nbits);
}



int64_t vebset_pred(vebset *self, uint32_t x)
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
    if (self->nbits == 1) {
        if (x == 1 && self->min == 0) {
            //DEBUG("x=1 and min=0. return 0\n");
            return 0;
        } else {
            //DEBUG("x=%ld and min=%ld. no pred. return -1\n", x, self->min);
            return -1;
        }
    }
    if (vebset_empty(self)) {
        return -1;
    }
    assert(self->summary);
    assert(self->clusters);
    int64_t high = HIGH(x, self->nbits);
    int64_t low = LOW(x, self->nbits);

    vebset *cluster = NULL;
    if (hashmap_contains(self->clusters, &high)) {
        cluster = (vebset*) hashmap_get_rawptr(self->clusters, &high);
    }
    if (cluster &&  vebset_min(cluster) < low) {
        assert(vebset_min(cluster) < UNIV(self->nbits));
        // predecessor in the same cluster as x
        //DEBUG("pred in same cluster #%u as x. Looking therein\n", high);
        low = vebset_pred(cluster, low);
        assert(vebset_min(cluster) <= low);
        //DEBUG("found pred in in cluster #%u at low = %ld\n", high, low);
    } else {
        // predecessor not in the same cluster as x
        //DEBUG("pred not in same cluster #%u as x=%u. looking for its pred in summary\n", high, x);
        high = vebset_pred(self->summary, high);
        //DEBUG("summary indicated pred is in cluster #%ld\n", high);
        if (high >= 0) {
            assert(hashmap_contains(self->clusters, &high));
            cluster = (vebset*) hashmap_get_rawptr(self->clusters, &high);
            low = vebset_max(cluster);
            assert (low >= 0);
        } else {
            if (self->min < UNIV(self->nbits)) {
                assert (self->min < x);
                //DEBUG("This should mean no pred, but this vEB has a min=%ld < %u. Therefore returning %ld\n", self->min, x, self->min);
                return self->min;
            } else {
                return -1;
            }
        }
    } 
    int64_t index =  INDEX(high, low, self->nbits);
    return index;
}