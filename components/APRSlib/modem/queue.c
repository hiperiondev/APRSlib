/*!\file cppQueue.cpp
** \author SMFSW
** \copyright BSD 3-Clause License (c) 2017-2024, SMFSW
** \brief cppQueue handling library (designed on Arduino)
** \details cppQueue handling library (designed on Arduino)
**			This library was designed for Arduino, yet may be compiled without change with gcc for other purposes/targets
**/
/****************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
/****************************************************************/

#define QUEUE_INITIALIZED 0x5AA5U //!< Initialized cppQueue control value

static queue_t impl; //!< cppQueue implementation: FIFO LIFO
static bool ovw;          //!< Overwrite previous records when queue is full allowed
static bool dynamic;      //!< Set to true when queue is dynamically allocated
static size_t queue_sz;   //!< Size of the full queue
static size_t rec_sz;     //!< Size of a record
uint16_t rec_nb;   //!< number of records in the queue
static uint8_t *queue;    //!< cppQueue start pointer (when allocated)

static uint16_t in;   //!< number of records pushed into the queue
static uint16_t out;  //!< number of records pulled from the queue (only for FIFO)
uint16_t cnt;  //!< number of records not retrieved from the queue
uint16_t init; //!< set to QUEUE_INITIALIZED after successful init of the queue, 0 otherwise

static bool _isInitialized = false;
static bool _isEmpty = false;
static bool _isFull = false;
static uint16_t _getCount = 0;

/**************************/
/*** INTERNAL FUNCTIONS ***/
/**************************/
/*!	\brief Increment index
**	\details Increment buffer index \b pIdx rolling back to \b start when limit \b end is reached
**	\param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void __attribute__((nonnull, always_inline)) _inc_idx(uint16_t *const pIdx, const uint16_t end, const uint16_t start) {
    if (*pIdx < (end - 1U)) {
        (*pIdx)++;
    } else {
        *pIdx = start;
    }
}

/*!	\brief Decrement index
**	\details Decrement buffer index \b pIdx rolling back to \b end when limit \b start is reached
**	\param [in,out] pIdx - pointer to index value
**	\param [in] end - counter upper limit value
**	\param [in] start - counter lower limit value
**/
static inline void __attribute__((nonnull, always_inline)) _dec_idx(uint16_t *const pIdx, const uint16_t end, const uint16_t start) {
    if (*pIdx > start) {
        (*pIdx)--;
    } else {
        *pIdx = end - 1U;
    }
}

/*!	\brief get initialization state of the queue
**	\return cppQueue initialization status
**	\retval true if queue is allocated
**	\retval false is queue is not allocated
**/
inline bool __attribute__((always_inline)) cppQueue__isInitialized(void) {
    return (init == QUEUE_INITIALIZED) ? true : false;
}

/*!	\brief get emptiness state of the queue
**	\return cppQueue emptiness status
**	\retval true if queue is empty
**	\retval false is not empty
**/
inline bool __attribute__((always_inline)) cppQueue__isEmpty(void) {
    return (cnt == 0U) ? true : false;
}

/*!	\brief get fullness state of the queue
**	\return cppQueue fullness status
**	\retval true if queue is full
**	\retval false is not full
**/
inline bool __attribute__((always_inline)) cppQueue__isFull(void) {
    return (cnt == rec_nb) ? true : false;
}

/*!	\brief get number of records in the queue
**	\return Number of records stored in the queue
**/
inline uint16_t __attribute__((always_inline)) cppQueue__getCount(void) {
    return cnt;
}

/************************/
/*** PUBLIC FUNCTIONS ***/
/************************/
void queue_init(const size_t size_rec, const uint16_t nb_recs, const queue_t type, const bool overwrite, void *const pQDat, const size_t lenQDat) {
    init = 0;
    rec_nb = 0;       // rec_nb needs to be 0 to ensure proper push behavior when queue is not allocated
    ovw = 0;          // ovw needs to be 0 to ensure proper push behavior when queue is not allocated
    queue_flush(); // other variables needs to be 0 to ensure proper functions behavior when queue is not allocated

    const size_t size = nb_recs * size_rec;

    dynamic = (pQDat == NULL) ? true : false;

    if (dynamic) {
        queue = (uint8_t *)malloc(size);
    } else if (lenQDat >= size) {
        queue = (uint8_t *)pQDat;
    } else {
        queue = NULL;
    }

    if (queue != NULL) {
        queue_sz = size;
        rec_sz = size_rec;
        rec_nb = nb_recs;
        impl = type;
        ovw = overwrite;

        init = QUEUE_INITIALIZED;
    }
}

void queue_deinit(void) {
    if (_isInitialized && dynamic && (queue != NULL)) {
        free(queue);
    }
}

void queue_flush(void) {
    in = 0;
    out = 0;
    cnt = 0;
}

bool __attribute__((nonnull)) queue_push(const void *const record) {
    bool ret = true;

    if (_isFull) // No more records available
    {
        if (ovw) // cppQueue is full, overwrite is allowed
        {
            if (impl == FIFO) {
                _inc_idx(&out, rec_nb, 0); // as oldest record is overwritten, increment out
            }
        } else {
            ret = false;
        }
    } else {
        cnt++; // Increase records count
    }

    if (ret) {
        uint8_t *const pStart = queue + (rec_sz * in);
        memcpy(pStart, record, rec_sz);
        _inc_idx(&in, rec_nb, 0);
    }

    return ret;
}

bool __attribute__((nonnull)) queue_pop(void *const record) {
    bool ret = true;

    if (_isEmpty) // No records
    {
        ret = false;
    } else {
        const uint8_t *pStart;

        if (impl == FIFO) {
            pStart = queue + (rec_sz * out);
            _inc_idx(&out, rec_nb, 0);
        } else /* if (impl == LIFO) */
        {
            _dec_idx(&in, rec_nb, 0);
            pStart = queue + (rec_sz * in);
        }

        memcpy(record, pStart, rec_sz);
        cnt--; // Decrease records count
    }

    return ret;
}

bool __attribute__((nonnull)) queue_peek(void *const record) {
    bool ret = true;

    if (_isEmpty) // No records
    {
        ret = false;
    } else {
        const uint8_t *pStart;

        if (impl == FIFO) {
            pStart = queue + (rec_sz * out);
            // No change on out var as it's just a peek
        } else /*if (impl == LIFO)*/
        {
            uint16_t rec = in; // Temporary var for peek (no change on in with dec_idx)
            _dec_idx(&rec, rec_nb, 0);
            pStart = queue + (rec_sz * rec);
        }

        memcpy(record, pStart, rec_sz);
    }

    return ret;
}

bool queue_drop(void) {
    bool ret = true;

    if (_isEmpty) // No records
    {
        ret = false;
    } else {
        if (impl == FIFO) {
            _inc_idx(&out, rec_nb, 0);
        } else /*if (impl == LIFO)*/
        {
            _dec_idx(&in, rec_nb, 0);
        }

        cnt--; // Decrease records count
    }

    return ret;
}

bool __attribute__((nonnull)) queue_peekIdx(void *const record, const uint16_t idx) {
    bool ret = true;

    if ((idx + 1U) > _getCount) // Index out of range
    {
        ret = false;
    } else {
        const uint8_t *pStart;

        if (impl == FIFO) {
            pStart = queue + (rec_sz * ((out + idx) % rec_nb));
        } else /*if (impl == LIFO)*/
        {
            pStart = queue + (rec_sz * idx);
        }

        memcpy(record, pStart, rec_sz);
    }

    return ret;
}

bool __attribute__((nonnull)) queue_peekPrevious(void *const record) {
    const uint16_t idx = _getCount - 1U; // No worry about count - 1 when queue is empty, test is done by peekIdx
    return queue_peekIdx(record, idx);
}

bool queue_isInitialized(void) {
    return _isInitialized;
}

bool queue_isEmpty(void) {
    return _isEmpty;
}

bool queue_isFull(void) {
    return _isFull;
}

uint32_t queue_sizeOf(void) {
    return queue_sz;
}

uint16_t queue_getCount(void) {
    return _getCount;
}

uint16_t queue_getRemainingCount(void) {
    return rec_nb - cnt;
}
