#if	!defined(INCLUDED_GSFIFO)
#define	INCLUDED_GSFIFO	1
/**
   Copyright (C) 2011 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date:        April 2011

   This file is part of the Performance Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.
   */
#import <Foundation/NSObject.h>

@class NSArray;
@class NSCondition;
@class NSNumber;
@class NSString;
@class NSThread;


/** GSFIFO manages a first-in-first-out queue of items.<br />
 * Items in the queue are <em>NOT</em> retained objects ... memory management
 * is not the job of this class and it is not intended to be treated
 * as a 'collection', rather its role is intended to be that of an
 * inter-thread coordination mechanism.<br />
 * Instances of the GSFIFO class are intended to support the producer-consumer
 * model of processing.  The ideal example is that of a production line,
 * where you have a stream of items to be processed and while that processing
 * can be broken down into separate stages, they must be done in a particular
 * order.  The FIFO is used as the link betwen those stages, ensuring that
 * the required ordering is maintained even when separate threads handle
 * each stage.<br />
 * Where there is a single producer and a single consumer thread, a fast
 * lock-free algorthm is used to get/pu items from the FIFO.<br />
 * To minimise the overheads of using the FIFO, we provide inline functions
 * to support the addition of items in the single producer thread case and to
 * support the removal of items in the single consumer thread case.  When
 * operating that way, the overhead of using the FIFO is only a few CPU cycles
 * and it becomes reasonable to split sequentional processing into a long
 * series of small operations each handled by a separate thread (making
 * effective use of a multi-cpu machine).<br />
 * The FIFO may also be useful where you don't have a strictly sequential
 * process to manage, but some parts need to be sequential ... in these
 * cases it may make sense to have multiple consumers and/or producers.
 * In these cases, some locking is required and the use of the inline
 * functions is not allowed (you must call the -get and -put: methods.<br />
 */
@interface	GSFIFO : NSObject
{
@public
/* While the following instance variables are nominally public, they are in
 * fact only intended to be used by the provided inline functions ... you
 * should not access them directly in your own code.
 */
  volatile uint64_t	_head;
  volatile uint64_t	_tail;
  uint64_t		_getTryFailure;
  uint64_t		_getTrySuccess;
  uint64_t		_putTryFailure;
  uint64_t		_putTrySuccess;
  void			**_items;
  uint32_t		_capacity;
@private
  uint32_t		boundsCount;
  uint16_t		granularity;
  uint16_t		timeout;
  uint64_t		fullCount;		// Total waits for full FIFO
  uint64_t		emptyCount;		// Total waits for empty FIFO
  NSCondition		*condition;
  NSString		*name;
  NSTimeInterval	getWaitTotal;		// Total time waiting for gets
  NSTimeInterval	putWaitTotal;		// Total time waiting for puts
  NSTimeInterval	*waitBoundaries;	// Stats boundaries
  uint64_t		*getWaitCounts;		// Waits for gets by time
  uint64_t		*putWaitCounts;		// Waits for puts by time
  NSThread		*getThread;		// Single consumer thread
  NSThread		*putThread;		// Single consumer thread
}

/** Return statistics for all current GSFIFO instances.<br />
 * Statistics for FIFOs which are configued to be lock-free are empty
 * (listing the name only) except where we can safely obtain get or put
 * stats because the FIFOs consumer/producer thread is the same as the
 * current thread.
 */
+ (NSString*) stats;

/** Returns the approximate number of items in the FIFO.
 */
- (NSUInteger) count;

/** Reads up to count items from the FIFO into buf.
 * If block is YES, this blocks if necessary until at least one item
 * is available, and raises an exception if the FIFO is configured
 * with a timeout and it is exceeded.<br />
 * Returns the number of items actually read.
 */
- (unsigned) get: (void**)buf  count: (unsigned)count  shouldBlock: (BOOL)block;

/** Gets the next item from the FIFO, blocking if necessary until an
 * item is available.  Raises an exception if the FIFO is configured
 * with a timeout and it is exceeded.<br />
 * Implemented using -get:count:shouldBlock:
 */
- (void*) get;

/** <init/>
 * Initialises the receiver with the specified capacity (buffer size).<br />
 * If the granularity value is non-zero, it is treated as the maximum time
 * in milliseconds for which a -get or -put: operation will pause between
 * successive attempts.<br />
 * If the timeout value is non-zero, it is treated as the total time in
 * milliseconds for which a -get or -put: operation may block, and a
 * longer delay will cause those methods to raise an exception.<br />
 * If the multiProducer or multiConsumer flag is YES, the FIFO is
 * configured to support multiple producer/consumer threads using locking.<br />
 * The boundaries array is an ordered list of NSNumber objects containing
 * time intervals found boundaries of bands into which to categorise wait
 * time stats.  Any wait whose duration is less than the interval specified
 * in the Nth element is counted in the stat's for the Nth band.
 * If this is nil, a default set of bundaries is used.  If it is an empty
 * array then no time based stats are recorded.<br />
 * The name string is a unique identifier for the receiver and is used when
 * printing diagnostics and statistics.
 */
- (id) initWithCapacity: (uint32_t)c
	    granularity: (uint16_t)g
		timeout: (uint16_t)t
	  multiProducer: (BOOL)mp
	  multiConsumer: (BOOL)mc
	     boundaries: (NSArray*)a
		   name: (NSString*)n;

/** Initialises the receiver as a multi-producer, multi-consumer FIFO with
 * no timeout and with default stats gathering enabled.
 */
- (id) initWithCapacity: (uint32_t)c
		   name: (NSString*)n;

/** Writes up to count items from buf into the FIFO.
 * If block is YES, this blocks if necessary until at least one item
 * can be written, and raises an exception if the FIFO is configured
 * with a timeout and it is exceeded.<br />
 * Returns the number of items actually written.
 */
- (unsigned) put: (void**)buf  count: (unsigned)count  shouldBlock: (BOOL)block;

/** Adds an item to the FIFO, blocking if necessary until there is
 * space in the buffer.  Raises an exception if the FIFO is configured
 * with a timeout and it is exceeded.br />
 * Implemented using -put:count:shouldBlock:
 */
- (void) put: (void*)item;

/** Return any available statistics for the receiver.<br />
 */
- (NSString*) stats;

/** Return statistics on get operations for the receiver.<br />
 * NB. If the recever is not configured for multiple consumers,
 * this method may only be called from the single consumer thread.
 */
- (NSString*) statsGet;

/** Return statistics on put operations for the receiver.<br />
 * NB. If the recever is not configured for multiple producers,
 * this method may only be called from the single producer thread.
 */
- (NSString*) statsPut;

/** Checks the FIFO and returns the first available item or NULL if the
 * FIFO is empty.<br />
 * Implemented using -get:count:shouldBlock:
 */
- (void*) tryGet;

/** Attempts to put an item into the FIFO, returning YES on success or NO
 * if the FIFO is full.<br />
 * Implemented using -put:count:shouldBlock:
 */
- (BOOL) tryPut: (void*)item;
@end

/** Function to efficiently get an item from a fast FIFO.<br />
 * Returns NULL if the FIFO is empty.<br />
 * Warning ... only for use if the FIFO is NOT configured for multiple
 * consumers.
 */
static inline void*
GSGetFastNonBlockingFIFO(GSFIFO *receiver)
{
  if (receiver->_head > receiver->_tail)
    {
      void	*item;

      item = receiver->_items[receiver->_tail % receiver->_capacity];
      receiver->_tail++;
      receiver->_getTrySuccess++;
      return item;
    }
  receiver->_getTryFailure++;
  return NULL;
}

/** Function to efficiently get an item from a fast FIFO, blocking if
 * necessary until an item is available or the timeout occurs.<br />
 * Warning ... only for use if the FIFO is NOT configured for multiple
 * consumers.
 */
static inline void*
GSGetFastFIFO(GSFIFO *receiver)
{
  void	*item = GSGetFastNonBlockingFIFO(receiver);

  if (0 == item)
    {
      item = [receiver get];
    }
  return item;
}

/** Function to efficiently put an item to a fast FIFO.<br />
 * Returns YES on success, NO on failure (FIFO is full).<br />
 * Warning ... only for use if the FIFO is NOT configured for multiple
 * producers.
 */
static inline BOOL
GSPutFastNonBlockingFIFO(GSFIFO *receiver, void *item)
{
  if (receiver->_head - receiver->_tail < receiver->_capacity)
    {
      receiver->_items[receiver->_head % receiver->_capacity] = item;
      receiver->_head++;
      receiver->_putTrySuccess++;
      return YES;
    }
  receiver->_putTryFailure++;
  return NO;
}

/** Function to efficiently put an item to a fast FIFO, blocking if
 * necessary untile there is space in the FIFO or until the timeout
 * occurs.<br />
 * Warning ... only for use if the FIFO is NOT configured for multiple
 * producers.
 */
static inline void
GSPutFastFIFO(GSFIFO *receiver, void *item)
{
  if (NO == GSPutFastNonBlockingFIFO(receiver, item))
    {
      [receiver put: item];
    }
}

#endif
