/* Copyright (c) 2015 Scott Kuhl. All rights reserved.
 * License: This code is licensed under a 3-clause BSD license. See
 * the file named "LICENSE" for a full copy of the license.
 */

/** @file

    List provides an array-like mechanism for storing items.

    The list is automatically resized so that the capacity of the list
    always matches or exceeds the length of the list. Anything can be
    stored in the list, but all items in the list must be the same
    size. The code is written to include numerous checks and not
    written with a sole focus on speed or efficiency.

    For example, if you want to store an int in the list, you would use:

    <pre>
    list *l = list_new(10, sizeof(int));
    int i=4;
    list_append(l, &i);
    // retrieving values from the list:
    int *x = list_get(l, 0);
    printf("%d\n", *x);
    </pre>

    Note that the list will store a *copy* of the int inside of the
    list---the list does not store a list of pointers. If you want to
    make a list of pointers, you should pass a pointer to a pointer
    into list_set() or list_append();

    It is important to note that the list struct contains three
    variables that can be read but should not be changed by functions
    outside of list.c:

    list->length: Contains the index of the highest set item in the
    array plus 1. This does not necessarily mean that all of the items
    within the list are actually set. For example, if you create a
    empty list and then add an item at index 100, length will indicate
    that there list contains 101 items. The items stored at index 0
    through index 100 will not be initialized but would be available
    for use.

    list->capacity: The length of the list could grow up to this
    capacity without the need for any reallocations. list_append() and
    list_prepend() will double the capacity if you add an item and the
    current capacity is not sufficient---allowing you to frequently
    add items to the list without frequent reallocations. list_set()
    and other functions will only increment the capacity to be exactly
    large enough to contain the list.

    list->itemSize: Each item in the list must be the same size. This
    variable indicates the size of the item in bytes.


    @author Scott Kuhl
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int capacity;
	int length;
	int itemSize;
	int (*compar)(const void *, const void *);
	void *data;
} list;
	
list* list_new(int capacity, int itemSize, int (*compar)(const void *, const void *));
list* list_new_import(int numItems, int itemSize, int (*compar)(const void *, const void *), void *array);
	
int list_reset(list *l, int capacity, int itemSize, int (*compar)(const void *, const void *));
int list_reset_import(list *l, int length, int itemSize, int (*compar)(const void *, const void *), void *array);
void list_free(list *l);
list* list_copy(const list *l);

int list_get(const list *l, int index, void *result);
void* list_getptr(const list *l, int index);
int list_set(list *l, int index, void *item);

int list_remove(list *l, int index, void *result);
int list_remove_all(list *l, void *item);
int list_insert(list *l, int index, void *item);
	
int list_append(list *l, void *item);
int list_prepend(list *l, void *item);

int list_ensure_capacity(list *l, int capacity);
int list_set_capacity(list *l, int capacity);
int list_set_length(list *l, int length);
int list_reclaim(list *l);

int list_swap(list *l, int a, int b);
int list_reverse(list *l);
int list_move(list *l, int src, int dst, int count);

int list_index_compare(const list *l, int index, const void *item);
int list_find(const list *l, const void *item);
int list_count(const list *l, const void *item);
int list_sort(      list *l);
int list_bsearch(   list *l, const void *item);

int list_push(list *l, void *item);
int list_pop(list *l, void *result);
int list_peek(const list *l, void *result);


	
/* TODO:
   find the last item in the list, or second item in the list, etc.
   
   list_concat(list *a, list *b)
*/

int list_length(const list *l);
int list_capacity(const list *l);
	
void list_sanity_check(const list *l);
void list_print_stats(const list *l);

/* A list can also be treated as a set. */
int set_add(list *l, void *item);
int set_remove(list *l, void *item);

/* TODO: set_union() set_intersection() */

#ifdef __cplusplus
} // end extern "C"
#endif
