#ifndef STRVIEW_H
#define STRVIEW_H

#include "coretype.h"
#include "iter.h"
#include "trait.h"


/**
 * @file strview.h
 * @brief String View
 * @author Paulo Fonseca
 *
 * A String View is a lightweight object used to represent a
 * *read-only* substring of an existing string.
 */


/**
 * @brief String View format modifier to be used to print its represented
 * substring with printf-family functions
 *
 * Example:
 * ```C
 * strview sv;
 * // initialise the strview...
 * printf("str = %"SVFMT" len = %zu\n", SVPRT(sv), sv.len);
 */
#define SVFMT ".*s"


/**
 * @brief To be used together with SVFMT to print the (sub)string content
 * of @p S  with printf-family functions
 *
 * @see SVFMT
 */
#define SVPRT(S) (int)S.len, S.str


/**
 * @brief String View type.
 */
typedef struct strview {
	const char *str;  /**< A **non-owned** pointer to a source string */
	size_t len; /**< The length of the view */
} strview;


/**
 * @brief String View initializer from a given pointer and length.
 *
 * @param self A pointer to the memory to be initialised
 * @param src a pointer to the source string
 * @param len The length of the (sub)string
 *
 * @warning The strview has only non-owned references, so no
 * specific destructor is needed for a stack-allocated strview.
 */
void strview_init(strview *self, const char* src, size_t len);


/**
 * @brief String View constructor initializer from a given pointer
 * and length.
 *
 * @warning The strview is a lightweight object meant to be mostly
 * stack-allocated of stored in flat arrays. This constructor returns
 * a new heap-allocated strview. This should be avoided but this
 * constructor is included for completeness. The strview has only
 * non-owned references, so no  specific destruction is needed.
 * A heap-allocated strview can be disposed of with FREE .
 *
 * @see strview_init
 * @see FREE
 */
strview *strview_new(const char* src, size_t len);


/**
 * @brief String View initializer from a given pointer and substring range.
 *
 * @param self A pointer to the memory to be initialised
 * @param src a pointer to the source string
 * @param from The initial position of the substring/offset from the start position
 * @param to The final position of the substring (not included)
 */
void strview_init_from_range(strview *self, const char* src, size_t from, size_t to);


/**
 * @brief String View constructor from a given pointer and substring range.
 *
 * @warning The strview is a lightweight object meant to be mostly
 * stack-allocated of stored in flat arrays. This constructor returns
 * a new heap-allocated strview. This should be avoided but this
 * constructor is included for completeness. The strview has only
 * non-owned references, so no  specific destruction is needed.
 * A heap-allocated strview can be disposed of with FREE .
 *
 * @see strview_new_from_range
 * @see FREE
 */
strview *strview_new_from_range(const char* src, size_t from, size_t to);


/**
 * @brief Creates a new heap-allocated null-terminated C string from
 * a strview
 */
char *strview_to_new_cstr(strview *self);


/**
 * @brief Copies the contents of a string view to a char buffer and appends
 * a null '\0' char.
 *
 * @param self The source strview
 * @param dest The destination buffer
 *
 * @warning @p dest must be large enough to hold the length of @self plus
 * the extra '\0'. Bound checks are not ensured.
 */
void strview_cpy_to_cstr(strview *self, char *dest);


/**
 * @brief Returns the length of th strview.
 */
size_t strview_len(strview *self);


/**
 * @brief Compares the contents of the string view to a raw string
 * for equality.
 */
bool strview_eq_cstr(strview *self, const char *other);


/**
 * @brief Compares the contents of the string view to a raw string
 * for equality up to the first @p n positions.
 *
 * @return true iff the length of the strview is at least @p n and
 * its first @p n positions are equal to those of @p other.
 */
bool strview_eqn_cstr(strview *self, const char *other, size_t n);


/**
 * @brief String View Iterator type.
 *
 * A strview iterator is used to iterate over substrings of a source
 * strview, each represented by another strview.
 *
 * @see strview_iter_has_next
 * @see strview_iter_next
 */
typedef struct strview_iter strview_iter;


/**
 * @brief String View Iterator destructor.
 */
void strview_iter_free(strview_iter *self);


/**
 * @brief Does the iterator have any remaining element?
 */
bool strview_iter_has_next(strview_iter *self);


/**
 * @brief Gets the next substring view of the iterator.
 * @return If  `strview_iter_has_next(it) == true` returns the next
 * substring view. Otherwise, the behaviour is undefined.
 */
strview strview_iter_next(strview_iter *self);


/**
 * @brief Tokenises a string view.
 *
 * Let `X` be a string of lenght `n >= 0` and `S` be a separator string
 * of length `m > 0` such that `X = T_0 S_1 T_1 S_2 ... T_k-1 S_k T_k`
 * where `S_1 .. S_k` are all the non-overlapping left-to-right occurrences
 * of `S` in `X` and `T_0 .. T_k` are the possibly empty strings in-between
 * these occurrences. This operation returns an iterator over `T_0 .. T_k`.
 *
 * Example
 *
 * ```C
 * strview sview;
 * strview_init(&sview, "strview_iter_has_next_" 22);
 * strview_iter *iter = strview_tok(&sview, "_", 1);
 * for (int i = 0; strview_iter_has_next(iter); i++) {
 *     strview tok = strview_iter_next(iter);
 *     printf("tok = %"SVFMT" (len = %zu)\n", SVPRT(tok), tok.len);
 *	}
 * ```
 * prints:
 * ```
 * tok = strview (len = 7)
 * tok = iter (len = 4)
 * tok = has (len = 3)
 * tok = next (len = 4)
 * tok =  (len = 0)
 * ```
 *
 * @param self The string view to be tokenised
 * @param sep The separator string
 * @param sep_len The length of the separator
 *
 * @returns An iterator over the tokens
 */
strview_iter *strview_tok(strview *self, const char *sep, size_t sep_len);


#endif
