#ifndef DEQUE_H
#define DEQUE_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @file deque.h
 * @author Paulo Fonseca
 *
 * Double-eneded queue (deque) ADT
 */


/**
 * Deque type
 */
typedef struct _deque deque;


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 */
deque *deque_new(size_t typesize);


/**
 * @brief Constructor.
 * @param typesize The individual size of stored elements (in bytes).
 * @param capacity The initial capacity.
 */
deque *deque_new_with_capacity(size_t typesize, size_t capacity);


/**
 * @brief Destructor.
 * @param free_elts Indicates whether referenced elements should be freed.
 */
void deque_free(deque *q, bool free_elts);


/**
 * @brief Checks whether the deque is empty.
 */
bool deque_empty(deque *q);


/**
 * @brief Returns the length (logical size) of the deque.
 */
size_t deque_len(deque *q);


/**
 * @brief Pushes an element onto the back of the deque.
 */
void deque_push_back(deque *q, void *elt);


/**
 * @brief Pops the element from the front of a nonempty deque and copies its 
 *        value into @p dest. If @p dest is NULL, the value is discarded. 
 */
void deque_pop_back(deque *q, void *dest);


/**
 * @brief Pushes an element onto the back of the deque.
 */
void deque_push_front(deque *q, void *elt);


/**
 * @brief Pops the element from the front of a nonempty deque and copies its 
 *        value into @p dest. If @p dest is NULL, the value is discarded. 
 */
void deque_pop_front(deque *q, void *dest);


#define DEQUE_PUSH_BACK_DECL( TYPE )\
    void deque_push_back_##TYPE(deque *q, TYPE val);


#define DEQUE_POP_BACK_DECL( TYPE ) \
    TYPE deque_pop_back_##TYPE(deque *q);    


#define DEQUE_PUSH_FRONT_DECL( TYPE )\
    void deque_push_front_##TYPE(deque *q, TYPE val);

#define DEQUE_POP_FRONT_DECL( TYPE ) \
    TYPE deque_pop_front_##TYPE(deque *q);    


#define DEQUE_ALL_DECL( TYPE )\
DEQUE_PUSH_BACK_DECL(TYPE) \
DEQUE_POP_BACK_DECL(TYPE)\
DEQUE_PUSH_FRONT_DECL(TYPE)\
DEQUE_POP_FRONT_DECL(TYPE)


DEQUE_ALL_DECL(int)
DEQUE_ALL_DECL(size_t)



#endif