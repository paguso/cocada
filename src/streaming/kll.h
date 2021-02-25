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

#ifndef KLL_H
#define KLL_H

#include <stdio.h>

#include "coretype.h"
#include "new.h"
#include "order.h"


/**
 * @file kll.h
 * @brief KLL Summary for ordered data
 * @author Paulo Fonseca
 *
 * A KLL summary of a sample `X` of size `N` consists in a sequence of buffers
 * ```
 * B[0] B[1] ... B[h]
 * ```
 * which store data points in increasing resolution. Each point at level `l`
 * represents `2^(h-l)` original points (for simplicity, lets assume that
 * each `B[l]` is an array). So, each point in `B[0]` is a point in the original
 * sample, each point in `B[1]` represents 2 original points,
 * each point in `B[2]` represents 4 original points, and so forth.
 *
 * Each buffer `B[l]` has a limited capacity `C[l] = k * c^(h-l)` for constants
 * `k` and `c`, s.t. `0.5 < c < 1`. The constant `k` depends on the desired
 * error level `err`, as we´ll see later. For now it suffices to note that
 * `k = C[h]`, that is, the capacity of the last (lowest resolution) buffer.
 * Because `c` liew between `1/2` and `1`, the capacities decay "smoothly" from
 * `k` to 2 (the minimum buffer capacity for technical reasons) from the
 * last to the first buffer.
 *
 * The rank of a value `v` in a sample `X`, defined as
 * ```
 * rank(v) = # {y in X | y < v}
 * ```
 * can be estimated from the KLL summary of `X` as
 * ```
 * r(v) = sum_l=0^h r_l(v) * c^l
 * ```
 * where
 * ```
 * r_l(v) = # { y in B[l] | y < v },
 * ```
 * the idea being that each point smaller than `v` in buffer `B[l]` represents
 * `c^l` points smaller than `v` in the original sample.
 * Of course the decreasing resolution of `B[l]` for `l>0` can introduce errors,
 * but it can be shown that we can ensure
 * ```
 * Pr [ |r_l(v)-rank(v)| > err * N ] = O(err),
 * ```
 * for some fixed error level `err`, by setting
 * ```
 * k = O( 1/err * sqrt( log( 1/err ) ) ).
 * ```
 * Intuitively, we have to increase the buffer capacity in a rate
 * roughly inversely proportional to the intended error level.
 *
 * The overall KLL summary capacity will be limited by a constant,
 * ```
 * C[0] + ... + C[h]  <=  k / (1-c) ,
 * ```
 * which means that `c` also regulates the overall space, and consequently
 * the actual error level (more space, less error).
 *
 * Adding a point to `KLL(X)` goes as follows. The point is initially inserted
 * in `B[0]`. If the maximum capacity `C[0]` is reached, then half of its points,
 * namely either the points at the odd or at the even positions are moved into
 * `B[1]`, and the remaining points in `B[0]` are deleted. Which points are
 * moved is decided by a fair coin toss. By the end of this first
 * 'compression', only about half of the original `B[0]` points remain in the
 * summary, but since they are now in `B[1]`, each of these points will
 * represent two of the original points, so that the overall number of
 * represented points remains about the same. Now, if the newcoming elements
 * cause `B[1]` to overflow, the process is repeated from `B[1]` to `B[2]`,
 * and so on until a level with enough space is reached. If the compression
 * reaches the last level `h` and even `B[h]` is full, then a new level
 * `B[h+1]` of capacity `k` is created to accomodate `k/2` points, and the
 * process is guaranteed to terminate successfully.
 *
 * Now consider the compression from `B[l]` to `B[l+1]`, let
 * ```
 * B[l] = b_0 b_1 b_2 ... b_{C[l]-1},
 * ```
 * and pick an arbitrary position `j`. If `j` is even, exactly half of
 * the elements to the left of `b_j` are moved, so that its contribution
 * to `r(v)` is maintained for whatever `v`. If `j` is odd, then either
 * a bit more or a bit less than half of the elements to the left of
 * `b_j` are moved, each event occurring with equal probability.
 * In average, the positive and negative differences to
 * the ranks cancel out, and as a result the expected value
 * of `r(v)` over all coin tosses is exactly `rank(v)` for arbitrary `v`.
 * In other words, the KLL summary provides for an unbiased rank estimator.
 *
 * The error analysis involves some Chernoff-Hoeffding upper
 * bounding, but as mentioned above, we can have a `reasonable'
 * approximation with constant probability.
 *
 */

/**
 *@brief Opaque KLL summary type.
 */
typedef struct __kllsumm kllsumm;

/**
 * @brief Default minimum KLL summary capacity (nb. of physically stored data points)
 */
static const size_t KLL_DEFAULT_CAP = 1024;

/**
 * @brief Default value of `c` constant (see kll.h module documentation)
 */
static const double KLL_DEFAULT_C = 0.75;

/**
 * @brief Default big-Oh multiplicative constant for `k`
 * @see kll.h module docs
 * @see kll_new
 * @see kll_new_with_cap
 */
static const double KLL_MIN_K_BIG_OH_CONST = 2.0;


/**
 * Creates a new KLL summary with no a priori space constraints
 * for *NON-OWNED* data.
 *
 * @param typesize the size of the stored elements in bytes
 * @param cmp comparison function
 * @param err The desired error level (see header file comments)
 *
 * This constructor will set the constant `k` to
 * ```
 * k = KLL_MIN_K_BIG_OH_CONST  * (1/err) * sqrt(log(1/err))
 * ```
 * and the other relevant constant `c` to KLL_DEFAULT_C.
 *
 * @warning This constructor *SHOULD NOT* be used if the summary
 * is supposed to contain *onwed* object references. If such is the
 * case, use kll_new_own or kll_new_onw_with_cap. This is equivalent
 * to kll_new_own(typesize, cmp, err, finaliser_new_empty())
 *
 * @see order.h
 * @see kll_new_own
 * @see kll_new_with_cap
 *
 */
kllsumm *kll_new(size_t typesize, cmp_func cmp, double err);


/**
 * @brief Same as kll_new but for summaries storing owned references.
 *
 * @param typesize the size of the stored elements in bytes
 * @param cmp comparison function
 * @param err The desired error level (see header file comments)
 * @param chd_fr (**move**) A destructor for the stored child objects
 *
 * The KLL summary stores the added data in internal containers.
 * If owned references are stored in the summary, then it needs to
 * know how to properly destroy such objects because some of these
 * references may have to be deleted during the lifecycle of the
 * summary, even before its destruction.
 *
 * @see new.h
 */
kllsumm *kll_new_own(size_t typesize, cmp_func cmp, double err,
                     finaliser *chd_fr);


/**
 * @brief Creates a new KLL summary with a given maximum capacity
 * i.e. (maximum number of stored data points) for *NON-OWNED*
 * data.
 *
 * @param typesize the size of the stored elements in bytes
 * @param cmp comparison function
 * @param err The desired error level (see header file comments)
 * @param cap The intended capacity
 *
 * First, recall that the overall capacity is roughly bounded by
 * ```
 * C[0] + .. + C[h] <=  k/(1-c).
 * ```
 * So, given an error level @p err and intended capacity @p cap,
 * this constructor sets the value of `c` to a default `intermediate`,
 * implementation-defined value (say `c=0.75`) and sees if we can have
 * ```
 * k/(1-c) = cap
 * ```
 * with
 * ```
 * k = q * (1/err) * sqrt(log(1/err)) = O((1/err)*sqrt(log 1/err))),
 * ```
 * as required, for a constant `q` at least as big as some minimum acceptable
 * value DEFAULT_K_CONST >= 1 (also implementation-defined).  If this is
 * possible, then `k` is set to such value and `c` is left at its default.
 *
 * Else, it tries to push `c` as far as possible towards `1/2`, while fixing
 * `k` at the minimum `acceptable level' by setting the big-Oh constant
 * `q` to KLL_MIN_K_BIG_OH_CONST. If no such `c > 0.5` can be found, an
 * error occurs.
 *
 * @warning As explained, an error can occur if the informed capacity is
 * insufficient to assure the error level.
 *
 * @warning If @p cap it is smaller than KLL_DEFAULT_CAP, the later value
 * is used, i.e. the capacity is always at least KLL_DEFAULT_CAP.
 *
 * @warning The capacity is an indicator of the maximum number of
 * data points are that are physically stored. However, the implementation
 * will require additional memory depending on the actual data structures.
 *
 * @warning This constructor *SHOULD NOT* be used if the summary
 * is supposed to contain *onwed* object references. If such is the
 * case, use  kll_new_onw_with_cap. This is equivalent
 * to kll_new_own_with_cap(typesize, cmp, err, finaliser_new_empty())
 *
 * @see errlog.h
 */
kllsumm *kll_new_with_cap(size_t typesize, cmp_func cmp, double eps,
                          size_t cap);


/**
 * @brief Same as kll_new_with_cap but for summaries storing owned references.
 *
 * @see kll_new_own
 * @see kll_new_with_cap
 */
kllsumm *kll_new_own_with_cap(size_t typesize, cmp_func cmp, double eps,
                              size_t cap, finaliser *chd_fr);


/**
 * @brief Destructor
 * If this summary is going to store
 * @see new.h
 */
void kll_finalise(void *ptr, const finaliser *fnr);


/**
 * @brief Updates the summary with a new a data point with a given value.
 */
void kll_upd(kllsumm *self, void *val);

/**
 * @brief Returns an `O(err*N)` approximation of the rank of @p val
 * with constant `O(err)` error probability.
 * @see kll.h module documentation
 */
size_t kll_rank(kllsumm *self, void *val);

/**
 * @brief Prints a representation of the summary to a given output stream
 * @param print_val A function to print the stored values.
 */
void kll_print(kllsumm *self, FILE *stream, void (*print_val)(FILE *,
               const void *));


#endif