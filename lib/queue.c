/* Copyright (c) 2014 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file
 * @author Scott Kuhl
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"
#include "msg.h"

static int queue_index_wrap(const queue *q, int index)
{
	return index % q->l->length;
}


void queue_sanity_check(const queue *q)
{
	if(q == NULL)
	{
		msg(MSG_FATAL, "q was NULL");
		exit(EXIT_FAILURE);
	}

	/* All of the checks below shouldn't ever be triggered unless
	   someone is changing the internal data stored in the queue in
	   inappropriate ways. The code in this file should keep all of the
	   variables set to appropriate values. */
	list_sanity_check(q->l);

	if(q->read == -1 && q->length > 0)
	{
		msg(MSG_FATAL, "Read pointer is at -1, but the queue apparently contains items.");
		exit(EXIT_FAILURE);
	}
	if(q->read > -1 && q->length == 0)
	{
		msg(MSG_FATAL, "Read pointer is at an index but the queue doesn't seem to contain any items.");
		exit(EXIT_FAILURE);
	}
	if(q->read >= q->l->length)
	{
		msg(MSG_FATAL, "Read pointer points at a location past the end of the list.");
		exit(EXIT_FAILURE);
	}
	if(q->write >= q->l->length)
	{
		msg(MSG_FATAL, "q->write=%d points at a location past the end of the list (length %d).", q->write, q->l->length);
		exit(EXIT_FAILURE);
	}
	if(q->read < -1 || q->write < 0)
	{
		msg(MSG_FATAL, "q->read or q->write are too small.");
		exit(EXIT_FAILURE);
	}
	if(q->read < -1 || q->read >= q->l->length)
	{
		msg(MSG_FATAL, "Queue read index is invalid.");
		exit(EXIT_FAILURE);
	}
	if(q->write < -1 || q->read >= q->l->length)
	{
		msg(MSG_FATAL, "Queue read index is invalid.");
		exit(EXIT_FAILURE);
	}
	if(q->l->capacity != q->l->length)
	{
		msg(MSG_FATAL, "List capacity and length differ");
		exit(EXIT_FAILURE);
	}
	if(q->length > 0 && queue_index_wrap(q, q->read + q->length) != q->write)
	{
		msg(MSG_FATAL, "The positions of the read and write pointer don't match the length of the queue. expectWrite=%d", queue_index_wrap(q, q->read + q->length));
		// queue_print_stats(q);
		exit(EXIT_FAILURE);
	}
}

/** Creates a new queue with enough capacity to store 'capacity' items
    which are each itemSize bytes.
    
    @param capacity Number of items that the queue should be able to
    hold initially.

    @param itemSize The size of each item to be stored in the queue.

    @return A pointer to a queue struct or NULL if we failed to
    allocate the queue. The queue should eventually be free'd with
    queue_free().
*/
queue* queue_new(int capacity, int itemSize)
{
	capacity++; /* increase capacity by 1 since we always need an
	             * empty spot to have our write pointer to point at */
	
	list *l = list_new(capacity, itemSize, NULL);
	queue *q = malloc(sizeof(queue));
	if(l == NULL || q == NULL)
	{
		if(l != NULL) free(l);
		if(q != NULL) free(q);
		msg(MSG_ERROR, "Failed to allocate queue");
		return NULL;
	}
	q->l = l;
	q->read = -1;
	q->write = 0;
	q->length = 0;
	list_set_length(q->l, capacity);
	queue_sanity_check(q);
	return q;
}


/** Resets an existing queue struct. The new queue will have the
    specified capacity and itemSize. The queue length will be set to
    0. The data stored in the queue will be lost.

    @param q The queue to reset.
    
    @param capacity Number of items that we should allocate space in
    the list for.

    @param itemSize The size of each item to be stored in the list.

    @param 1 if success, 0 if failure
*/
int queue_reset(queue *q, int capacity, int itemSize)
{
	capacity++; /* increase capacity by 1 since we always need an
	             * empty spot to have our write pointer to point at */

	if(q == NULL)
	{
		msg(MSG_ERROR, "Failed to reset a queue because it is NULL");
		return 0;
	}

	if(q->l == NULL)
	{
		msg(MSG_ERROR, "The queue does not have a valid list.");
		return 0;
	}

	if(list_reset(q->l, capacity, itemSize, NULL) == 0)
	{
		msg(MSG_ERROR, "Failed to set up a list for the queue.");
		return 0;
	}
	q->read = -1;
	q->write = 0;
	q->length = 0;
	list_set_length(q->l, capacity);
	queue_sanity_check(q);
	return 1;
}

/** Frees a queue object created by queue_new().

    @param q The queue to be free()'d
 */
void queue_free(queue *q)
{
	if(q != NULL)
	{
		if(q->l != NULL)
			list_free(q->l);
		free(q);
	}
}



/** Adds an item to the queue. If the capacity of the queue is too
    small, it will be doubled.

    @param q The queue that the item will be added to.

    @param item The item to add to the queue.
    
    @return 1 if successful, 0 if failure. Failure can occur if we
    needed to expand the capacity of the queue and we failed to do
    so. Failure can also occur if the queue or the item is NULL.
*/
int queue_add(queue *q, void *item)
{
	queue_sanity_check(q);

	if(item == NULL)
	{
		msg(MSG_ERROR, "The pointer to the item to add to the queue was NULL.");
		return 0;
	}

	/* The list must always have one spot empty that the q->write
	index corresponds to. So, we add two our current length (count the
	new item we are adding + the blank space that we need) and check
	that it fits in the capacity of the current list. */
	if(q->length + 2 > q->l->capacity)
	{
		// If we need to expand the capacity of our queue, double it.
		int origCapacity = queue_capacity(q);
		queue_set_capacity(q, origCapacity*2);
	}

	/* Try adding item into the appropriate index */
	if(list_set(q->l, q->write, item) == 0)
	{
		msg(MSG_ERROR, "Failed to add item into he queue.");
		return 0;
	}

	/* If the queue was empty previously, set the read index to the
	 * index that we just wrote to. */
	if(q->read < 0)
		q->read = q->write;

	/* Move write so it points at the next location */
	q->write = queue_index_wrap(q, q->write+1);
	q->length++;
	return 1;
}

/** Removes an item from the queue and optionally updates a parameter
    to contain a copy of the data that was removed.

    Does not change the capacity of the queue.

    @param q The queue to remove an item from.
    
    @param result A copy of the item that was removed from the
    queue. There must be enough space allocated where item is pointing
    at to store the removed item. If result == NULL, a copy of the item
    is not made.
    
    @return 1 if successful, 0 if failure.
*/
int queue_remove(queue *q, void *result)
{
	queue_sanity_check(q);

	if(queue_peek(q, result) == 0)
	{
		msg(MSG_ERROR, "Failed to remove an item from the queue because we couldn't read an item from it.");
		return 0;
	}

	/* If the read index caught up with the write index, the queue is
	 * empty. */
	int nextRead = queue_index_wrap(q, q->read + 1);
	if(nextRead == q->write)
		q->read = -1;
	else
		q->read = nextRead;
	
	q->length--;
	queue_sanity_check(q);
	return 1;
}

/** Retrieve a copy of the item in the queue that would be removed next.

    @param q The queue to retrieve the item from.
    @param result A place to copy the item into.
    @return 1 if success, 0 if failure
*/
int queue_peek(queue *q, void *result)
{
	if(q->read < 0)
		return 0; // queue is empty

	/* Get a copy of the item if one was requested. */
	if(result != NULL)
		if(list_get(q->l, q->read, result) == 0)
			return 0;

	return 1;
}

/** Allocates the minimum sized space for the given queue. Adding a
    single item to the queue will result in the queue needing to be
    reallocated.

    @param q The queue to reclaim unused space from.

    @return 1 if successful, 0 if fail.
*/
int queue_reclaim(queue *q)
{
	queue_sanity_check(q);

	int newCapacity = q->length;
	if(newCapacity < 4)
		newCapacity = 4;

	// queue_set_capacity will account for the extra slot we need to
	// store the blank spot for the write pointer to point at.
	queue_set_capacity(q, newCapacity);
	queue_sanity_check(q);
	return 1;
}


void queue_print_stats(const queue *q)
{
	printf("Queue information\n");
	printf("Items in queue:    %4d item(s)\n", q->length);
	printf("Write location:    %4d index\n", q->write);
	printf("Read location:     %4d index\n", q->read);
	printf("Capacity:          %4d item(s)\n", q->l->length-1);
	printf("Internal capacity: %4d item(s)\n", q->l->length);
	// list_print_stats(q->l);
}

/** Returns the length of the queue. The caller could also access this
    value via q->length.

    @param q The queue which the caller wants to know the length of.

    @return The length of the queue or -1 if failure.
*/
int queue_length(const queue *q)
{
	if(q == NULL)
		return -1;
	return q->length;
}

/** Returns the capacity of the queue. The queue will automatically be
 * resized to a large capacity as needed.

    @param q The queue which the caller wants to know the capacity of.

    @return The capacity of the queue or -1 if failure.
*/
int queue_capacity(const queue *q)
{
	if(q == NULL)
		return -1;
	if(q->l == NULL)
		return -1;

	// The capacity of the list is always one more than the capacity
	// of the queue because the list will always contain at least one
	// empty spot for q->write to point to.
	return q->l->capacity-1;
}


/** Increases the capacity of the queue to be at least the specified capacity.

    @param q The queue that we want to change the capacity of.
    
    @param capacity The capacity that the caller wants to ensure that
    the queue has.
    
    @return 1 if successful, 0 if failure.
*/
int queue_ensure_capacity(queue *q, int capacity)
{
	if(q == NULL)
		return 0;
	if(queue_capacity(q) >= capacity)
		return 1;

	return queue_set_capacity(q, capacity);
}

/** Sets the capacity of the queue to a specific size.

    @param q The queue that we want to change the capacity of.

    @param capacity The new capacity of the queue.
    
    @return 1 if successful, 0 if failure.
*/
int queue_set_capacity(queue *q, int capacity)
{
	if(queue_capacity(q) == capacity)
		return 1;

	/* Don't allow the capacity to fall under the length of the queue. */
	if(capacity < q->length)
		return 0;
	
	if(capacity < 4)
		capacity = 4;

	/* It is easy to change the capacity if the queue is empty */ 
	if(q->read == -1)
	{
		//printf("resizing empty queue\n");
		if(list_set_capacity(q->l, capacity+1) == 0)
			return 0;
		if(list_set_length(q->l, capacity+1) == 0)
			return 0;
		/* If we reduced the size of the queue, the write pointer may
		 * point off the end of the list. Reset it to 0. */
		q->write = 0;

		if(queue_capacity(q) != capacity)
		{
			msg(MSG_FATAL, "internal queue error: capacity wasn't set correctly\n");
			exit(EXIT_FAILURE);
		}
		queue_sanity_check(q);
		return 1;
	}

	/* If the queue is wrapped around the right edge of the list, we
	 * need to do the same thing if we are expanding or shrinking the
	 * capacity: Keep everything starting at the read pointer pushed
	 * up against the far right side of the new resized list. */
	if(q->write < q->read)
	{
		// printf("queue is wrapped\n");
		int numItemsToMove = q->l->capacity - q->read;
		int dst = capacity+1 - numItemsToMove;

		// list_move will increase the capacity of the list if needed.
		if(list_move(q->l, q->read, dst, numItemsToMove) == 0)
			return 0;
		q->read = dst;

		// If capacity of the queue decreased, we need to explicitly set it.
		if(capacity < queue_capacity(q))
		{
			if(list_set_capacity(q->l, capacity+1) == 0)
				return 0;
			if(list_set_length(q->l, capacity+1) == 0)
				return 0;
		}
		if(queue_capacity(q) != capacity)
		{
			msg(MSG_FATAL, "internal queue error: capacity wasn't set correctly\n");
			exit(EXIT_FAILURE);
		}
		queue_sanity_check(q);
		return 1;
	}

	else if(q->write > q->read) // queue is not wrapped
	{
		// printf("queue is not wrapped\n");
		
		if(capacity < queue_capacity(q))  // capacity is being reduced
		{
			// If the queue capacity is also the largest index allowed
			// within a list (because the list capacity is always one
			// greater than the queue capacity)
			if(q->write > capacity) // if data covers the indices to be truncated, move it as far left as possible.
			{
				if(list_move(q->l, q->read, 0, q->length) == 0)
					return 0;
				q->read = 0;
				q->write = q->length;
			}
		}

		// When we get here, we can safely expand the list or shrink
		// the list without fear of having data being truncated in the
		// process.
		if(list_set_capacity(q->l, capacity+1) == 0)
			return 0;
		if(list_set_length(q->l, capacity+1) == 0)
			return 0;
		
		if(queue_capacity(q) != capacity)
		{
			msg(MSG_FATAL, "internal queue error: capacity wasn't set correctly\n");
			exit(EXIT_FAILURE);
		}
		queue_sanity_check(q);
		return 1;
	}

	msg(MSG_FATAL, "Internal error: We should not reach this point\n");
	exit(EXIT_FAILURE);
}
