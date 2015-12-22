/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */


/** @file
 * @author Scott Kuhl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> /* int32_t */

/* For random number generation: */
#include <time.h>      // time()
#ifndef _WIN32
#include <unistd.h>    // getpid()
#endif
#include <sys/types.h> // getpid()


#include "list.h"
#include "msg.h"

#define LIST_MIN_CAPACITY 4  /*< The smallest allowable capacity of a list */


/** Verify that the list is valid. You should not need to call this
 * unless you are modifying the the list object yourself. */
void list_sanity_check(const list *l)
{
	if(l == NULL)
	{
		msg(MSG_FATAL, "l was NULL");
		exit(EXIT_FAILURE);
	}

	/* All of the checks below shouldn't ever be triggered unless
	   someone is changing the length, itemSize, or capacity of the
	   list in an inappropriate way. The code in list.c should keep
	   all of the variables set to appropriate values. */
	if(l->capacity < LIST_MIN_CAPACITY)
	{
		msg(MSG_FATAL, "l->capacity=%d is smaller than the smallest allowed capacity (%d)",
		    l->capacity, LIST_MIN_CAPACITY);
		exit(EXIT_FAILURE);
	}
	if(l->length < 0)
	{
		msg(MSG_FATAL, "l->length=%d was negative", l->length);
		exit(EXIT_FAILURE);
	}
	if(l->itemSize <= 0)
	{
		msg(MSG_FATAL, "l->itemSize=%d was zero or negative", l->length);
		exit(EXIT_FAILURE);
	}
	if(l->capacity < l->length)
	{
		msg(MSG_FATAL, "l->capacity=%d was less than l->length=%d", l->capacity, l->length);
		exit(EXIT_FAILURE);
	}
}

/** Creates a new list with enough capacity to store 'capacity' items
    which are each itemSize bytes.
    
    @param capacity Number of items that we should allocate space in
    the list for.

    @param itemSize The size of each item to be stored in the list.

    @return A pointer to a list struct or NULL if we failed to
    allocate the list. The list should eventually be free'd with
    list_free().
*/
list* list_new(int capacity, int itemSize, int (*compar)(const void *, const void *))
{
	list *l = malloc(sizeof(list));
	if(l == NULL)
	{
		msg(MSG_ERROR, "Failed to allocate space for a list which can hold %d %d byte items.", capacity, itemSize);
		return NULL;
	}
	l->data = NULL;
	if(list_reset(l, capacity, itemSize, compar) == 0)
		return NULL;
	return l;
}

/** Creates a new array with numItems capacity and initializes with
    a copy of the data in the given array.

    @param numItems The number of items in the array and the list to be created.
    @param itemSize The size of the items in the array and the list to be created.
    @param array The data to initialize the list to.
    @return A new list or NULL if failure
*/
list* list_new_import(int numItems, int itemSize, int (*compar)(const void *, const void *), void *array)
{
	list *l = malloc(sizeof(list));
	if(l == NULL)
	{
		msg(MSG_ERROR, "Failed to allocate space for a list which can hold %d %d byte items.", numItems, itemSize);
		return NULL;
	}
	l->data = NULL;
	if(list_reset_import(l, numItems, itemSize, compar, array) == 0)
		return NULL;
	return l;
}

/** Allocates space and exports the data contained in the list into a
    new array. The original list structure is left unchanged.

    @param l The list to export to an array.
    
    @return A newly allocated array containing a copy of the items in
    the list. The returned pointer can be cast by the caller into the
    type of data that the array contains. The returned pointer should
    be free()'d by the caller. Returns NULL if a failure occurs.
*/
void* list_new_export(const list *l)
{
	int bytes = l->itemSize * l->length;
	void *ptr = malloc(bytes);
	if(ptr == NULL)
		return NULL;
	memcpy(ptr, l->data, bytes);
	return ptr;
}


/** Copies the data from the list and stores it into space allocated
    by the caller. The original list structure is left unchanged.

    @param l The list to export to an array.

    @param result The location that the data should be exported
    to. The caller is responsible for ensuring that there is enough
    space to store l->length * l->itemSize bytes at the location that
    this pointer points at.

    @return 1 if success, 0 if failure.
*/
int list_export(const list *l, void *result)
{
	if(l == NULL || result == NULL)
		return 0;
	memcpy(result, l->data, l->length * l->itemSize);
	return 1;
}


/** Resets an existing list struct. The new list will have the
    specified capacity and itemSize. The list length will be set to
    0. Assume that the data in the list will be lost.

    @param l The list to reset. If list->data==NULL, a new space for
    data will be allocated.
    
    @param capacity Number of items that we should allocate space in
    the list for.

    @param itemSize The size of each item to be stored in the list.

    @return 1 if success, 0 if failure.
*/
int list_reset(list *l, int capacity, int itemSize, int (*compar)(const void *, const void *))
{
	if(l == NULL)
	{
		msg(MSG_ERROR, "Failed to reset a list because it is NULL");
		return 0;
	}

	if(capacity < LIST_MIN_CAPACITY)
		capacity = LIST_MIN_CAPACITY;

	if(itemSize < 1)
	{
		msg(MSG_FATAL, "itemSize was 0 or negative\n");
		exit(EXIT_FAILURE);
	}

	// Fill in information about the list
	l->capacity = capacity;
	l->itemSize = itemSize;
	l->length   = 0;
	l->compar   = compar;
	
	/* If the data isn't NULL, we are assuming that this is an old
	 * list that we are reusing. We don't need to malloc new
	 * space---just realloc space that is the appropriate size. */
	if(l->data != NULL)
	{
		if(list_set_capacity(l, l->capacity) == 0)
		{
			/* If we fail to realloc enough space, free the space and
			 * set l->data to NULL */
			free(l->data);
			l->data = NULL;
		}
	}
	else
		// If a new list, allocate space.
		l->data = malloc(l->capacity * l->itemSize);

	if(l->data == NULL)
	{
		msg(MSG_ERROR, "Unable to allocate space for list.");
		return 0;
	}

	list_sanity_check(l);
	return 1;
}

/** Resets the list to contain a copy of the items in the given
    array. The capacity of the list will match the length of the
    provided array.

    @param l The list to be modified.
    
    @param numItems The number of items in the array that is to be
    copied into the list. Also, this will be the initial capacity of
    the list.

    @param itemSize The size of each item in the array (and in the
    list that is going to be set up).

    @param array An array containing items to be copied into the list.

    @return 1 if success, 0 if failure.
 */
int list_reset_import(list *l, int numItems, int itemSize, int (*compar)(const void *, const void *), void *array)
{
	/* reset the list so it has exactly the capacity needed */
	if(list_reset(l, numItems, itemSize, compar) == 0)
		return 0;

	memcpy(l->data, array, itemSize * numItems);
	l->length = numItems;
	l->compar = compar;
	return 1;
}

/** Frees a list object created by list_new().

    @param l The list to be free()'d
 */
void list_free(list *l)
{
	if(l != NULL)
	{
		if(l->data != NULL)
			free(l->data);
		free(l);
	}
}

/** Appends an item onto the end of the list (at index l->length). If
    the capacity of the list is too small, it will be doubled.

    @param l The list to append an item onto.

    @param item The item to append to the end of the list.
    
    @return 1 if successful, 0 if failure. Failure can occur if we
    needed to expand the capacity of the list and we failed to do
    so. Failure can also occur if the list or the item is NULL.
*/
int list_append(list *l, void *item)
{
	list_sanity_check(l);

	if(item == NULL)
	{
		msg(MSG_ERROR, "The pointer to the item to append to the list was NULL.");
		return 0;
	}

	/* Make sure that we double the capacity when needed. list_set()
	 * would also allocate more space for us---but will only increase
	 * the capacity just as large as needed. */
	if(l->length == l->capacity)
		if(list_ensure_capacity(l, l->capacity*2) == 0)
			return 0;

	return list_set(l, l->length, item);
}

/** Prepends an item into the list (slow!).  If the capacity of the
    list is too small, it will be doubled.

    @param l The list to the prepend the item to.

    @param item A pointer to an item to prepend onto the list.

    @return 1 if successful, 0 if failure. Failure can occur if we
    needed to expand the capacity of the list and we failed to do
    so. Failure can also occur if the list or the item is NULL.
*/
int list_prepend(list *l, void *item)
{
	return list_insert(l, 0, item);
}

/** Removes an item from the list and optionally updates a parameter
    to contain a copy of the data that was removed. Any items to the
    right of the removed index are shifted one index to the left. The
    length of the list is reduced by 1 and the capacity will be unchanged.

    Does not change the capacity of the list.

    @param l The list to remove an item from.
    
    @param index The index of the item to be removed.
    
    @param result A copy of the item that was removed from the
    list. There must be enough space allocated where item is pointing
    at to store the removed item (similar to list_get()). If result ==
    NULL, a copy of the item is not made.
    
    @return 1 if successful, 0 if failure.
*/
int list_remove(list *l, int index, void *result)
{
	list_sanity_check(l);
	
	if(index < 0 || index >= l->length)
	{
		msg(MSG_ERROR, "Can't remove index %d when list is of length %d",
		    index, l->length);
		return 0;
	}

	/* Get a copy of the item if one was requested. */
	if(result != NULL)
		if(list_get(l, index, result) == 0)
			return 0;
	
	/* If we are removing the last item in the list, we just need to
	 * update the length field and we are done. */
	if(index == l->length-1)
	{
		l->length--;
		return 1;
	}

	if(list_move(l, index+1, index, l->length - index - 1) == 0)
		return 0;
	l->length--;
	list_sanity_check(l);
	return 1;
}

/** Repeatedly looks for the specified item in the list using
    list_find() and removes all of the matching items.

    @param l The list to remove the items from
    @param item Remove all items that match this one
    @return 1 if success, 0 if fail.
 */
int list_remove_all(list *l, void *item)
{
	int found = list_find(l, item);
	while(found >= 0)
	{
		list_remove(l, found, NULL);
		found = list_find(l, item);
	}
	if(found == -2)
		return 0; // fail
	if(found == -1)
		return 1; // success

	return 0; // fail 
}

/** Checks if storing an item at this index would require increasing
    the capacity of the list.

    @param l The list in question.
    
    @param index The index that we would like to set.

    @return 1 if an item can be added at the index without increasing
    the list capacity, 0 otherwise.
*/
int list_index_need_realloc(const list *l, int index)
{
	if(l == NULL)
		return 0;
	if(index < 0 || index >= l->capacity)
		return 0;
	return 1;
}

/** Checks if adding an item at the specified index would increase the
    length of the list.

    @param l The list.

    @param index The index where an item might be added.

    @return 1 if adding an item at the specified index would increase
    the length of the list. 0 otherwise.
*/
int list_index_increases_length(const list *l, int index)
{
	if(l == NULL)
		return 0;
	if(index < 0 || index >= l->length)
		return 0;
	return 1;
}

/** Copies a contiguous chunk of items from one location to another
    location within the list. Expands the length and/or capacity to
    store the copy of the data but does not decrease the length or
    capacity of the list. The original copy of the data will be
    untouched.

    @param l The list that a section should be copied from.

    @param src The index of the first item that should be copied.

    @param dst The destination index that the first item should be
    copied to.

    @param count The number of items starting at index src that are to

    @return 1 is success, 0 if failure.

 */
int list_move(list *l, int src, int dst, int count)
{
	list_sanity_check(l);

	/* Neither the indices nor the count should be negative */
	if(src < 0 || dst < 0 || count < 0)
		return 0;

	if(src == dst || count == 0)
		return 1;

	/* The entire area that we are supposed to copy should be within
	 * the bounds of the existing list. */
	if(src >= l->length && src+count > l->length)
		return 0;
	
	/* If the destination goes past the end of the list, increase the
	 * length if needed. */
	if(dst + count > l->length)
		list_set_length(l, dst+count); // also increases the capacity is needed.

	void *srcPtr = list_getptr(l, src);
	void *dstPtr = list_getptr(l, dst);
	if(srcPtr == NULL || dstPtr == NULL)
	{
		msg(MSG_FATAL, "srcPtr or dstPtr were NULL, this shouldn't happen because %d and %d should be in bounds in an list of length %d", src, dst, l->length);
		exit(EXIT_FAILURE);
	}

	/* Move the data. memmove allows areas of memory to be
	 * overlapped. */
	memmove(dstPtr, srcPtr, count * l->itemSize);

	list_sanity_check(l);
	return 1;
}


/** Inserts an item into a specific index in the list. Moves the
    existing items at that index or at a higher index to the right. If
    needed, it will increase the capacity of the list to match the new
    length of the list.

    @param l The list to insert an item into.
    
    @param index The index to put the new item. If index == length,
    the item is appended to the end of the array. If index is negative
    or greater than the length of the list, an error occurs.

    @param item A pointer to the item to store in the list.
    
    @return 1 if successful, 0 otherwise. Failure can occur if we
    needed to allocate more space but failed to, if the index was not
    valid.
 */
int list_insert(list *l, int index, void *item)
{
	list_sanity_check(l);
	
	/* If we are inserting the item at the end of the list */
	if(index == l->length)
		return list_set(l, index, item);

	if(list_move(l, index, index+1,
	             l->length - index) == 0)
		return 0;

	/* Set the new item */
	return list_set(l, index, item);
}

/** Retrieves the data stored in the list at index and stores a copy
    of the data at the location that result points to. The area that
    result points to must be sufficiently large to store an item from
    the list (l->itemSize) contains the number of bytes of each item
    in the list.

    @param l The list to retrieve an item from.

    @param index The location of the item in the list.
    
    @param result A pointer to the area of memory where a copy of the
    item should be stored. The space must be sufficiently large for
    the item.

    @return 1 if successful, 0 if fail.
*/
int list_get(const list *l, int index, void *result)
{
	// Note: list_getptr() calls list_sanity_check().
	void *ptr = list_getptr(l, index);
	if(ptr != NULL && result != NULL)
	{
		if(ptr != result)
			memcpy(result, ptr, l->itemSize);
		return 1;
	}
	else
	{
		msg(MSG_FATAL, "Failed to get item from index %d. The list has length %d\n", index, l->length);
		exit(EXIT_FAILURE);
	}
}

/** Gets a pointer to a specific index into the list. list_get() is
    recommended unless you have a good reason to use list_getptr().

    @param l The list to get the item from.

    @param index The index of the item to get

    @return A pointer to the item or NULL if the index didn't exist in
    the list. IMPORTANT: Do not free() the returned item. This is a
    pointer to a specific index into the list. Changing the memory
    that the pointer points at will change the value in the
    list. Assume that the pointer will no longer be valid after any
    additional list operation that is not list_getptr(). For example,
    if you call list_getptr() and then list_append(), the
    list_append() may have caused the entire list to be stored at an
    entirely new location in memory---making the pointer returned by
    list_getptr() to be completely wrong. If you call list_getptr()
    and then list_prepend(), the list may have moved or your pointer
    may be pointing at a different item in the list.
*/
void* list_getptr(const list *l, int index)
{
	if(l == NULL)
		return NULL;
	
	if(index < 0 || index >= l->length)
		return NULL;

	list_sanity_check(l);
	
	// void* arithmetic is not allowed except as a GCC extension. Cast to char*:
	char *d = (char*) l->data;
	return (void*) (d + index * l->itemSize);
}

/** Sets an item in the list. If more space needs to be allocated, the
    capacity of the list is updated so that it is just large enough to
    hold the new item.

    @param l The list that the item should be added into.
    
    @param index The index to store the item in.
    
    @param item A pointer to the item to store.
    
    @return 1 if successful, 0 if failure. Failure can occur if we
    needed to expand the list and were unable to expand it.
*/
int list_set(list *l, int index, void *item)
{
	list_sanity_check(l);
	if(list_ensure_capacity(l, index+1) == 0)
		return 0;

	/* Update the length of our list if necessary. We need to do this
	 * before we call list_getptr() so that it will find the index to
	 * be in bounds. */
	if(l->length < index+1)
		l->length = index+1;
	
	// void* arithmetic is not allowed except as a GCC extension. Cast to char*:
	void *dest = list_getptr(l, index);
	if(dest != item)
		memcpy(dest, item, l->itemSize);

	return 1;
}


/** Sets the capacity of the list to a larger or smaller value. If you
    set the capacity to a value that is smaller than the length of the
    list, the list will be truncated and the length variable will be
    updated.

    @param l The list to resize.

    @param capacity The new capacity of the list. This must be 1 or
    higher. Values smaller than 1 are interpreted as as 1.
    
    @return 1 if successful, 0 if failed. If this function fails, the
    capacity of the list will be unchanged.
*/
int list_set_capacity(list *l, int capacity)
{
	list_sanity_check(l);

	if(capacity < LIST_MIN_CAPACITY)
		capacity = LIST_MIN_CAPACITY;

	void* newData = realloc(l->data, capacity * l->itemSize);

	if(newData == NULL)
	{
		msg(MSG_ERROR, "Unable to increase list capacity from %d items to %d items", l->capacity, capacity);
		return 0;
	}

	// if(newData != l->data)
	//	printf("realloc moved our data!\n");
	l->capacity = capacity;

	/* If we reduced the capacity smaller than the number of items
	 * already in the list. */
	if(l->capacity < l->length)
		l->length = l->capacity;

	l->data = newData;
	list_sanity_check(l);
	return 1;
}

/** Ensures that the list has a capacity of at least the given
    capacity. Same as list_set_capacity() except that this function
    guarantees that we will never reduce the capacity of the list.

    @param l The list which we want to ensure has a certain capacity.

    @param capacity We want the list to have a capacity of at least
    this amount.

    @return 1 if successful, 0 if failed.
*/    
int list_ensure_capacity(list *l, int capacity)
{
	if(capacity <= 0)
		return 1;
	if(capacity > l->capacity)
		return list_set_capacity(l, capacity);
	else
		return 1;
}

/** Sets the length of the list to a specific value and increases the
    capacity to match the length (if the existing capacity is too
    small). This function does *not* reduce the amount of space
    allocated (i.e., the 'capacity') for the list if the length of the
    list is decreased. list_reclaim() or list_set_capacity() can be
    used to reduce the capacity.

    @param l The new length of the list.
    
    @return 1 if successful, 0 if failed. Failure can happen if the
    list is NULL, the specified length is negative, or if we need to
    expand the capacity of the list and failed to do so.
 */
int list_set_length(list *l, int length)
{
	if(l == NULL || length < 0)
		return 0;

	if(list_ensure_capacity(l, length) == 0)
		return 0;

	l->length = length;
	return 1;
}

/** Sets the capacity of the list to match the current length of the
    list. This function is useful if you know that the list won't grow
    further and you want to eliminate any "extra" space that has been
    allocated. This does not prevent the the capacity of the list from
    changing again in the future.

    @param l The list to reclaim unused space.
    
    @return 1 if successful, 0 if failed.
 */
int list_reclaim(list *l)
{
	return list_set_capacity(l, l->length);
}


/** Creates a copy of a list with the same data, length, capacity, and
    itemSize.

    @param l The list to copy.
    
    @return A copy of the list which should eventually be deleted with
    list_free(). Returns NULL if NULL is passed in.
*/
list* list_copy(const list *l)
{
	if(l == NULL)
		return NULL;

	list_sanity_check(l);
	list *newl = list_new(l->capacity, l->itemSize, l->compar);
	list_set_length(newl, l->length);
	memcpy(newl->data, l->data, l->length*l->itemSize);
	list_sanity_check(newl);
	return newl;
}


/** Swaps the locations of two items in the list.

    @param l The list to swap two items in.
    
    @param a The index of the first item to swap.
    
    @param b The index of the second item two swap.
    
    @return 1 on success, 0 on failure. Failure can happen if the
    indices are inappropriate or if l is NULL.
*/
int list_swap(list *l, int a, int b)
{
	list_sanity_check(l);
	if(l == NULL)
		return 0;
		
	if(a==b)
		return 1;

	void *aPtr = list_getptr(l, a);
	void *bPtr = list_getptr(l, b);
	if(aPtr == NULL || bPtr == NULL)
	{
		msg(MSG_ERROR, "Can't swap indices %d and %d in a list of length %d\n", a, b, l->length);
		return 0;
	}

	void *tmp = malloc(l->itemSize);
	if(tmp == NULL)
	{
		msg(MSG_ERROR, "Failed to allocate space to swap indices %d and %d\n", a, b);
		return 0;
	}
	memcpy(tmp,  aPtr, l->itemSize);
	memcpy(aPtr, bPtr, l->itemSize);
	memcpy(bPtr, tmp,  l->itemSize);
	free(tmp);
	return 1;
}

/** Reverses the order of items in a list (in place).

    @param The list to reverse.

    @return 1 if success, 0 if failure. Failure can occur if we failed
    to allocate temporary space to facilitate swapping the location of
    items in the array. If there is a failure, the list may be in a
    partially reversed state.
*/
int list_reverse(list *l)
{
	/* No need to reverse an empty list or a list with only one
	 * item. */
	if(l->length < 2)
		return 1;
		
	for(int i=0; i < l->length/2; i++)
	{
		int otherEnd = l->length-i-1;
		if(list_swap(l, i, otherEnd) == 0)
		{
			msg(MSG_ERROR, "Failed to reverse the list.");
			return 0;
		}
	}

	return 1;
}

/** This list data structure can be used as a stack. Pushing an item
 * onto the stack is the same as appending it. */
int list_push(list *l, void *item)
{
	return list_append(l, item);
}

/** This list data structure can be used as a stack. Popping an item
 * is the same as removing an item from the end of the list. */
int list_pop(list *l, void *result)
{
	return list_remove(l, l->length-1, result);
}

/** This list data structure can be used as a stack. Peeking at the
 * stack gives you a copy of the top item on the stack. */
int list_peek(const list *l, void *result)
{
	if(l->length == 0)
		return 0;
	return list_get(l, l->length-1, result);
}


void list_print_stats(const list *l)
{
	printf("List information\n");
	printf("Items in list:     %4d item(s)\n", l->length);
	printf("Capacity of list:  %4d item(s)\n", l->capacity);
	printf("Unused capacity:   %4d item(s)\n", l->capacity - l->length);
	printf("Size of each item: %4d byte(s)\n", l->itemSize);
	printf("Space allocated:   %4d bytes + %4d bytes\n", l->itemSize * l->capacity, (int) sizeof(list));
}

/** Returns the length of the list. The caller could also access this
    value via l->length.

    @param l The list which the caller wants to know the length of.

    @return The length of the list or -1 if failure.
*/
int list_length(const list *l)
{
	if(l == NULL)
		return -1;
	return l->length;
}

/** Returns the capacity of the queue. The list will automatically be
    resized to a large capacity as needed. The caller could also
    access this value via l->capacity.

    @param q The queue which the caller wants to know the capacity of.

    @return The capacity of the queue or -1 if failure.
*/
int list_capacity(const list *l)
{
	if(l == NULL)
		return -1;
	return l->capacity;
}

/** Search the list for the first matching item in the
    list. Comparison uses the memcmp() function if l->compar is NULL.

    @param l The list to search.

    @param item The item to find.

    @return The index of the item. -1 if the item was not found. -2 if
    there was an error.
 */
int list_find(const list *l, const void *item)
{
	if(l == NULL)
		return -2; // error

	for(int i=0; i<l->length; i++)
	{
		int compareResult = list_index_compare(l, i, item);
		if(compareResult == -1)
			return -2; // error
		else if(compareResult == 1)
			return i;  // found
	}
	return -1; // not found
}

/** Counts the number of times the specified item appears in the
    list. Comparison uses the memcmp() function if l->compar is NULL.

    @param l The list to search.

    @param item The item to find.

    @return Returns the number of times the item appears in the
    list. Returns a negative number on error.
 */
int list_count(const list *l, const void *item)
{
	if(l == NULL)
		return -1; // error
	int count = 0;

	for(int i=0; i<l->length; i++)
	{
		int compareResult = list_index_compare(l, i, item);
		if(compareResult == -1)
			return -1; // error
		else if(compareResult == 1)
			count++;  // found
	}
	return count;
}


/** Sorts the items in the list. l->compar must be set.

   @param l The list to sort.

   @return 1 if successful, 0 if fail
*/
int list_sort(list *l)
{
	if(l == NULL || l->compar == NULL)
		return 0;

	qsort(l->data, l->length, l->itemSize, l->compar);
	return 1;
}

/** Uses a binary search to find an item in a sorted list. l->compar
    must be set.

   @param l A sorted list.

   @param item The item to find.
   
   @return The index of the matching item (if found), -1 if not found,
   -2 if failure
*/
int list_bsearch(list *l, const void *item)
{
	if(l == NULL || l->compar == NULL || item == NULL)
		return -2;

	char *ptr = (char*) bsearch(item, l->data, l->length, l->itemSize, l->compar);
	if(ptr == NULL)
		return -1;
	return (ptr - (char*)l->data) / l->itemSize;
}


/** Generates random integers in the range of [min,max]. This function
 * uses lrand48() which can only generate numbers 2^31 numbers from
 * [0,2^31-1]. Therefore, this function will fail if the range between
 * min and max is too large.
 *
 * @param min The smallest value to return
 * @param max The largest value to return
 * @return A random number between min and max (inclusive). Exits on failure.
 */
int32_t list_rand_interval(int32_t min, int32_t max)
{
#ifdef _WIN32
	msg(MSG_ERROR, "This function is not defined on windows.");
	return 0;
#else
	/* Although srand48() might have been called elsewhere, make sure
	 * that we call it at least once to seed the generator. */
	static int srand_done = 0;
	if(srand_done == 0)
	{
		// http://stackoverflow.com/questions/8056371
		srand48((getpid()*2654435761U)^time(NULL));
		srand_done = 1;
	}

	if(max < min)
	{
		int32_t tmp = min;
		min = max;
		max = tmp;
	}
	else if(min == max)
		return min;
	
	const int32_t range   = 1 + (max - min);
	if(range < 1)
	{
		msg(MSG_FATAL, "Invalid range for random number generator (too large?). min=%d max=%d\n", min, max);
		exit(EXIT_FAILURE);
	}
	const int32_t buckets = 2147483647 / range;
	const int32_t limit   = buckets * range;
		
	long r;
	do { r = lrand48(); } while( r >= limit );
	return (int32_t) (min + r/buckets);
#endif
}

/** Randomly shuffles all of the items in the list. Exits on failure,
    immediately returns without exiting if l is NULL.

    @param l The list to shuffle.
 */
void list_shuffle(list *l)
{
	if(l == NULL)
		return;
	
	// https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
	for(int i=l->length-1; i>=1; i--)
	{
		int j = list_rand_interval(0, i); // index to swap
		if(list_swap(l, i, j) == 0)
		{
			msg(MSG_FATAL, "Internal list error while shuffling\n");
			exit(EXIT_FAILURE);
		}
	}
}

/** Checks if the an item is at an index in a list. Comparison uses
    the memcmp() function if l->compar is NULL.

    @param l A list

    @param index The index to look at in the list

    @param item The item to be compared against the item at the index.

    @returns 1 if the item matches the item at the specified index, 0
    if it does not match, -1 on failure.
 */
int list_index_compare(const list *l, int index, const void *item)
{
	if(l == NULL || item == NULL)
		return -1; // error
	void *tmp = list_getptr(l,index);
	if(tmp == NULL)
		return -1; // error
	if(l->compar == NULL)
	{
		if(memcmp(tmp, item, l->itemSize) == 0)
			return 1;  // match
		else
			return 0; // no match
	} else {
		if(l->compar(tmp, item) == 0)
			return 1; // match
		else
			return 0; // no match
	}
}



/** If the item is not already in the list, add it.

    @param l The list which contains the items in a set.
    @param item The item to add to the set.
    @return 1 if success, 0 if failure.
 */
int set_add(list *l, void *item)
{
	if(l == NULL || item == NULL)
		return 0;

	int foundResult = list_find(l, item);
	if(foundResult == -1) // not found
	{
		if(list_append(l, item) == 1)
			return 1; // successfully added to set
		else
			return 0; // error adding item to set
	}
	else if(foundResult == -2)
		return 0; // error finding item in the list
	else
		return 1; // item was found in the list
}

/** If an item is in the list, remove it. If the item appears multiple
    times in the list, only the first item will be removed (which
    would be the case if all items are added using set_add() ). Use
    list_remove_all() to remove all matching items from the list.

    @param l The list to remove an item from.
    @param item The item to remove.
    @return 1 if success, 0 if failure
*/
int set_remove(list *l, void *item)
{
	if(l == NULL || item == NULL)
		return 0;

	int foundResult = list_find(l, item);
	if(foundResult == -1)
		return 1; // success, item wasn't in set.
	else if(foundResult == -2)
		return 0; // error
	else
	{
		if(list_remove(l, foundResult, NULL) == 0)
			return 0;
		return 1;
	}
}
