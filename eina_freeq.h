/*
 * Linked lists support
 *
 * Copyright (C) 2002 Alexandre Julliard
 * Copyright (C) 2011 Mike McCormack (adapted for Eina)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef EINA_FREEQ_H_
#define EINA_FREEQ_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"

/**
 * @addtogroup Eina_FreeQ_Group Free queue group.
 * @ingroup Eina.
 *
 * @brief This provides a mechanism to defer the freeing of memory
 * to some time in the future. Driven by the EFL main loop, 
 * the main free queue ensures that data is eventually freed.
 *
 * For debugging and tuning free queues of the default type, set 
 * the following environment variables:
 *
 * EINA_FREEQ_BYPASS=1/0
 *
 * Set this environment variable to 1 to immediately bypass the free queue, and
 * to have all items submitted with their free function immediately.
 * 
 * Set this environment variable to 0 to force the free queue to delay the freeing of items.
 * 
 * @note You can override this by setting count or mem max via
 * eina_freeq_count_max_set() or eina_freeq_mem_max_set(), which will
 * disable the bypass for that specific free queue. Once the bypass is disabled,
 * it cannot be re-enabled.
 *
 * EINA_FREEQ_FILL_MAX=N
 *
 * Sets the maximum number, in bytes, to N. An item in the free queue may
 * be filled with debugging values like 0x55 so you can see what memory has been 
 * freed and what memory has not been freed.
 * 
 * @note This value is one greater than the actual maximum. So, if 
 * the value is set to 100, a memory blob of 99 bytes will be filled.
 *
 * EINA_FREEQ_TOTAL_MAX=N
 *
 * Sets the maximum number of items allowed to N by
 * default before emptying the free queue to make room.
 *
 * EINA_FREEQ_MEM_MAX=N
 *
 * Sets the maximum number of allowed memory to N Kb (kilobites) of data.
 *
 * EINA_FREEQ_FILL=N
 *
 * Sets the byte value of every byte added to the free queue.
 * The default value is 0x55 (85). Memory is only filled if the size of
 * the allocation is less than the maximum. You can adjust the
 * maximum with EINA_FREEQ_FILL_MAX.
 *
 * EINA_FREEQ_FILL_FREED=N
 *
 * Fills every byte of memory with this pattern value
 * just before it is freed. The default value is 0x77 (119).
 * Memory is only filled if the size of the allocation is
 * less than the maximum. You can adjust the maximum 
 * with EINA_FREEQ_FILL_MAX.
 *
 * @since 1.19
 * 
 * @typedef Eina_FreeQ
 *
 * A queue of pointers to free in the future. You can create custom free
 * queues to defer freeing using the main free queue, which frees
 * memory as it iterates. Otherwise, eina will free everything on shutdown.
 * 
 */
typedef struct _Eina_FreeQ Eina_FreeQ;

/** @brief Types of free queues.
 * @since 1.19
 */
typedef enum _Eina_FreeQ_Type
{
   
/** @brief Default free queue.
*
* In a default free queue, any object added should be considered freed
* immediately. You should only use this type of free queue for debugging or 
* for additional memory safety.
*
* As this type of free queue is thread-safe, the free functions used must
* also be thread-safe (e.g.. libc free()).
*
* @since 1.19
*/

EINA_FREEQ_DEFAULT,

/** @brief Postponed free queue.
*
* Postponed free queues behave differently since objects added to them
* are not freed immediately. Rather, they are short-lived. Use this type 
* of free queue to return temporary objects that may only be used in a local 
* scope. The lifetime of the queued objects ends as soon as the execution 
* comes back to the loop. Objects added to this type of free queue should 
* be accessed exclusively from the thread that adds them.
*
* If a thread does not have a loop attached, the application may leak all
* the objects. At the time this document was published, this means that
* the main loop should use this type of freeq queue.
*
* Queues with no memory limit will be entirely flushed when the execution 
* comes back to the loop.
*
* This type of free queue is not thread-safe, and should be considered local
* to a single thread.
*
* @since 1.19
*/
   
EINA_FREEQ_POSTPONED,
} Eina_FreeQ_Type;

/**
 * @brief Create a new free queue to defer freeing of data.
 * @return A new free queue.
 * @since 1.19
 */
EAPI Eina_FreeQ *
eina_freeq_new(Eina_FreeQ_Type type);

/**
 * @brief Free a free queue and anything that is queued within it.
 * @param[in,out] fq The free queue to free and clear.
 * @since 1.19
 */
EAPI void
eina_freeq_free(Eina_FreeQ *fq);

/**
 * @brief Query the type of free queue.
 * @param[in] fq The free queue to inspect.
 * @since 1.19
 */
EAPI Eina_FreeQ_Type
eina_freeq_type_get(Eina_FreeQ *fq);

/**
 * @brief Get the main loop free queue.
 * @return The main loop free queue.
 * @since 1.19
 */
EAPI Eina_FreeQ *
eina_freeq_main_get(void);

/**
 * @brief Set the maximum number of free pointers allowed in this queue.
 * @param[in,out] fq The free queue to alter.
 * @param[in] count The maximum number of items allowed. Negative values
 *            indicate "no limit."
 *
 * Alters the maximum number of pointers allowed in the given free
 * queue. If more items are added to the free queue than allowed,
 * excess items will be freed to make room for new items. If the count is
 * changed, then excess items may be cleaned out at the time this API is
 * called.
 *
 * @note Setting a maximum count on a postponed free queue leads to undefined
 *       behavior.
 *
 * @since 1.19
 */
EAPI void
eina_freeq_count_max_set(Eina_FreeQ *fq, int count);

/**
 * @brief Get the maximum number of free pointers allowed in this queue.
 * @param[in] fq The free queue to query.
 * @return The maximum number of free items allowed, or -1 for infinity.
 * @since 1.19
 */
EAPI int
eina_freeq_count_max_get(Eina_FreeQ *fq);

/**
 * @brief Set the maximum amount of memory allowed.
 * @param[in,out] fq The free queue to alter.
 * @param[in] mem The maximum memory in bytes.
 *
 * Alters the maximum amount of memory allowed for pointers stored
 * in the free queue. The size used is the size given, so items 
 * with a size of 0 will not contribute to this limit. If items with a total
 * memory footprint are added to the free queue, items will be cleaned out
 * until the total is below this limit. Changing the limit may involve
 * cleaning out excess items from the free queue until the total amount of
 * memory used is below or at the limit.
 *
 * @note Setting a memory limit on a postponed free queue leads to undefined
 *       behavior.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_mem_max_set(Eina_FreeQ *fq, size_t mem);

/**
 * @brief Get the maximum amount of memory allowed.
 * @param[in] fq The free queue to query.
 * @return The maximum amount of memory in bytes.
 *
 * @since 1.19
 */
EAPI size_t
eina_freeq_mem_max_get(Eina_FreeQ *fq);

/**
 * @brief Clear out all queued items to be freed by freeing them.
 * @param[in,out] fq The free queue to clear.
 *
 * Removes all queued items from the free queue.
 * When it returns, the free queue should be empty.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_clear(Eina_FreeQ *fq);

/**
 * @brief Reduce the number of items in the free queue by up to @p count.
 * @param[in,out] fq The free queue to reduce in item count.
 * @param[in] count The number of items to try to free.
 *
 * Attempts to free up to @p count items from the given free queue,
 * Thus, reduces the amount of memory it is holding onto. This function
 * will return once it has removed @p count items, or once there are no 
 * remaining items to remove from the queue.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_reduce(Eina_FreeQ *fq, int count);

/**
 * @brief Return if there are any items pending in the free queue.
 * @param[in] fq The free queue to query.
 * @return EINA_TRUE if there are items to free. Otherwise, EINA_FALSE.
 * 
 * @since 1.19
 */
EAPI Eina_Bool
eina_freeq_ptr_pending(Eina_FreeQ *fq);

/**
 * @brief Add a pointer with a free function and the size to the free queue.
 * @param[in,out] fq The free queue to add the pointer to.
 * @param[in] ptr The pointer to free.
 * @param[in] free_func The function used to free the pointer.
 * @param[in] size The size of the data the pointer points to.
 *
 * Adds the given @p ptr pointer to the queue that will be freed 
 * later on. The function @p free_func is used. If this is NULL, 
 * the libc free() function is used instead. 
 * 
 * The @p size parameter determines the size of the data pointed to, 
 * but if this is 0, then no assumptions are made regarding the size,
 * and the pointer is considered opaque.
 * 
 * A pointer with a size of 0 will not contribute to the total memory usage of
 * items in the queue. If @p size is supplied, it must be correct,
 * as the memory may be written for debugging purposes. Otherwise,
 * it must be inspected or checksummed. Once a pointer is added to the 
 * free queue with this API, the memory should be considered freed as if 
 * the real @p free_func was called. (It may actually be called
 * if certain environment variables are set.)
 * 
 * A free queue exists to move the cost of freeing to a more
 * convenient point in time. A free queue also exists to provide some 
 * robustness for badly written code that may access memory after freeing.
 * 
 * @note When using tools like valgrind, eina will immediately free
 * the data so valgrind's own memory checkers can be deployed.
 *
 * @note The free function must not be used to access the content of the 
 * memory to be freed and consider it to be invalid or to be full of garbage.
 * It is already invalid at the moment it is added to the free queue. Only
 * the free function may be delayed.
 * 
 * @note The free function must not access memory that has already 
 * been freed before being added to the free queue. However, the free 
 * function may perform tricks such as using memory headers outside of the memory 
 * region to be freed (pass in pointer char *x, then char *header_address = x - 16 to get
 * header information), as this header is not considered part of the free data.
 * This note does not apply if you are using a size of 0 for the pointer.
 * Keep in mind that you lose the canary debugging ability when using pointers
 * with a size of 0 on the free queue.
 * 
 * @since 1.19
 */
EAPI void
eina_freeq_ptr_add(Eina_FreeQ *fq, void *ptr, void (*free_func) (void *ptr), size_t size);

/**
 * @brief Add a pointer to the main free queue.
 * @param[in] ptr The pointer to free.
 * @param[in] free_func The function used to free the pointer with.
 * @param[in] size The size of the data the pointer points to.
 * 
 * This is the same as eina_freeq_ptr_add(), but the main free queue is
 * fetched by eina_freeq_main_get().
 * 
 * @since 1.19
 */
static inline void
eina_freeq_ptr_main_add(void *ptr, void (*free_func) (void *ptr), size_t size)
{
   eina_freeq_ptr_add(eina_freeq_main_get(), ptr, free_func, size);
}

/**
 * @brief Convenience macro for well-known structures and types.
 * @param[in] ptr The pointer to free.
 * 
 * This is the same as eina_freeq_ptr_main_add(), but the free function is
 * assumed to be the libc free() function, and the size is provided by
 * sizeof(*ptr). Thus, it will not work on void pointers, or it will be inaccurate
 * for pointing to arrays. For arrays, please use EINA_FREEQ_ARRAY_FREE().
 * 
 * @since 1.19
 */
#define EINA_FREEQ_FREE(ptr) eina_freeq_ptr_main_add(ptr, NULL, sizeof(*(ptr)))

/**
 * @brief Convenience macro for well-known structures and types.
 * @param[in] ptr The pointer to free.
 * 
 * This is the same as eina_freeq_ptr_main_add(), but the free function is
 * assumed to be the libc free() function, and the size is provided by
 * sizeof(*ptr). Thus, it will not work on void pointers. The total size 
 * is multiplied by the count @p n, so it should work well for arrays of types.
 * 
 * @since 1.19
 */
#define EINA_FREEQ_N_FREE(ptr, n) eina_freeq_ptr_main_add(ptr, NULL, sizeof(*(ptr)) * n)

/**
 * @}
 */

#endif
