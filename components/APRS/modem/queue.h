/*!\file cppQueue.h
** \author SMFSW
** \copyright BSD 3-Clause License (c) 2017-2024, SMFSW
** \brief cppQueue handling library (designed on Arduino)
** \details cppQueue handling library (designed on Arduino)
**			This library was designed for Arduino, yet may be compiled without change with gcc for other purposes/targets
**/

#ifndef CPPQUEUE_H_
#define CPPQUEUE_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!\enum enumcppQueueType
** \brief cppQueue behavior enumeration (FIFO, LIFO)
**/
typedef enum {
    FIFO = 0, //!< First In First Out behavior
    LIFO = 1  //!< Last In First Out behavior
} queue_t;

/*!	\brief cppQueue constructor
**	\param [in] size_rec - size of a record in the queue
**	\param [in] nb_recs - number of records in the queue
**	\param [in] type - cppQueue implementation type: FIFO, LIFO
**	\param [in] overwrite - Overwrite previous records when queue is full
**	\param [in] pQDat - Pointer to static data queue
**	\param [in] lenQDat - Length of static data queue (in bytes) for static array size check against required size for queue
**	\return nothing
**/
void queue_init(const size_t size_rec, const uint16_t nb_recs, const queue_t type, const bool overwrite, void *const pQDat, const size_t lenQDat);

/*!	\brief cppQueue destructor: release dynamically allocated queue
 **/
void queue_deinit(void);

/*!	\brief Flush queue, restarting from empty queue
 **/
void queue_flush(void);

/*!	\brief Clean queue, restarting from empty queue
**	\deprecated clean was already used in cppQueue lib, alias is made to keep compatibility with earlier versions
**/
inline void queue_clean(void) {
    queue_flush();
}

/*!	\brief get initialization state of the queue
**	\return cppQueue initialization status
**	\retval true if queue is allocated
**	\retval false is queue is not allocated
**/
bool queue_isInitialized(void);

/*!	\brief get emptiness state of the queue
**	\return cppQueue emptiness status
**	\retval true if queue is empty
**	\retval false is not empty
**/
bool queue_isEmpty(void);

/*!	\brief get fullness state of the queue
**	\return cppQueue fullness status
**	\retval true if queue is full
**	\retval false is not full
**/
bool queue_isFull(void);

/*!	\brief get size of queue
**	\remark Size in bytes (like sizeof)
**	\return Size of queue in bytes
**/
uint32_t queue_sizeOf(void);

/*!	\brief get number of records in the queue
**	\return Number of records stored in the queue
**/
uint16_t queue_getCount(void);

/*!	\brief get number of records in the queue (same as getCount)
**	\deprecated nbRecs was already used in cppQueue lib, alias is made to keep compatibility with earlier versions
**	\return Number of records stored in the queue
**/
inline uint16_t queue_nbRecs(void) {
    return queue_getCount();
}

/*!	\brief get number of records left in the queue
**	\return Number of records left in the queue
**/
uint16_t queue_getRemainingCount(void);

/*!	\brief Push record to queue
**	\param [in] record - pointer to record to be pushed into queue
**	\return Push status
**	\retval true if successfully pushed into queue
**	\retval false if queue is full
**/
bool queue_push(const void *const record) __attribute__((nonnull));

/*!	\brief Pop record from queue
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\param [in,out] record - pointer to record to be popped from queue
**	\return Pop status
**	\retval true if successfully popped from queue
**	\retval false if queue is empty
**/
bool queue_pop(void *const record) __attribute__((nonnull));

/*!	\brief Pull record from queue (same as pop)
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\deprecated pull was already used in cppQueue lib, alias is made to keep compatibility with earlier versions
**	\param [in,out] record - pointer to record to be pulled from queue
**	\return Pull status
**	\retval true if successfully pulled from queue
**	\retval false if queue is empty
**/
inline bool queue_pull(void *const record) {
    return queue_pop(record);
}

/*!	\brief Peek record from queue
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\note This function is most likely to be used in conjunction with drop
**	\param [in,out] record - pointer to record to be peeked from queue
**	\return Peek status
**	\retval true if successfully peeked from queue
**	\retval false if queue is empty
**/
bool queue_peek(void *const record) __attribute__((nonnull));

/*!	\brief Drop current record from queue
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\note This method is most likely to be used in conjunction with peek
**	\return drop status
**	\retval true if successfully dropped from queue
**	\retval false if queue is empty
**/
bool queue_drop(void);

/*!	\brief Peek record at index from queue
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\note This function is only useful if searching for a duplicate record and shouldn't be used in conjunction with drop
**	\param [in,out] record - pointer to record to be peeked from queue
**	\param [in] idx - index of the record to pick
**	\return Peek status
**	\retval true if successfully peeked from queue
**	\retval false if index is out of range
**/
bool queue_peekIdx(void *const record, const uint16_t idx) __attribute__((nonnull));

/*!	\brief Peek previous record from queue
**	\warning If using push, pop, peek, drop, peekItem and/or peekPrevious in both interrupts and main application,
**				you shall disable interrupts in main application when using these functions
**	\note This inline is only useful with FIFO implementation, use peek instead with a LIFO (will lead to the same result)
**	\param [in,out] record - pointer to record to be peeked from queue
**	\return Peek status
**	\retval true if successfully peeked from queue
**	\retval false if queue is empty
**/
bool queue_peekPrevious(void *const record) __attribute__((nonnull));

#endif /* CPPQUEUE_H_ */
