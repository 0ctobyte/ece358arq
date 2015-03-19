#include "es.h"

#include <assert.h>
#include <stdlib.h>

// Root is at index 1. Index 0 is not used
#define ROOT 1

// Get index to right child in events array
#define RIGHT(index) (((index)<<1)+1)

// Get index to left child in events array
#define LEFT(index) ((index)<<1)

// Get parent of child at index
#define PARENT(index) ((index)>>1)

// Check if parent at index has children
#define HAS_CHILDREN(index, pq) (LEFT((index)) <= (pq)->size)

// Check if left child exists
#define HAS_LEFT_CHILD(index, pq) HAS_CHILDREN((index), (pq))

// Check if right child exists
#define HAS_RIGHT_CHILD(index, pq) (RIGHT((index)) <= (pq)->size)

// Macro to swap to events at the specified indices
#define SWAP_EVENTS(pq, index1, index2) es_event_t tmp = (pq)->events[(index1)]; \
										(pq)->events[(index1)] = (pq)->events[(index2)]; \
										(pq)->events[(index2)] = tmp;				

// Priority queue
struct es_pq {
	es_event_t *events;
	size_t size;
	size_t capacity;
};

// Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2	
// Rounds integer to next highest power of 2
uint64_t _es_pq_next_pow2(uint64_t num) {
	num--;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;
	return (num++);
}

// Resizes the event list to capacity specified
void _es_pq_resize(es_pq_t *pq, size_t capacity) {
	// Capacity must be greater than size or we lose information
	assert(capacity >= pq->size);

	// Allocate new event list
	pq->capacity = capacity;
	pq->events = (es_event_t*)realloc(pq->events, sizeof(es_event_t)*(pq->capacity+1));
	assert(pq->events != NULL);
}

es_pq_t* es_pq_create(size_t capacity) {
	// Allocate memory for the pq
	es_pq_t *pq = (es_pq_t*)malloc(sizeof(es_pq_t));
	assert(pq != NULL);
	
	// Round capacity to next highest power of 2 because power of 2 numbers are
	// easier to deal with kthnxbai
	pq->capacity = _es_pq_next_pow2(capacity);
	pq->size = 0;

	// The root of the heap is at index 1 (index 0 is not used), thus we need an array
	// with capacity+1
	pq->events = (es_event_t*)malloc(sizeof(es_event_t)*(pq->capacity+1));
	assert(pq->events != NULL);

	return pq;
}

void es_pq_enqueue(es_pq_t *pq, es_event_t ev) {
	// Check if events array is full, if so, double capacity
	if(pq->size >= pq->capacity) _es_pq_resize(pq, pq->capacity<<1);
	
	// Place the event at the end of the array
	pq->events[++(pq->size)] = ev;

	// Check if event.time is less than its parent, if so, swap
	// Do this until event.time is greater than its parent or we have reached root	
	for(uint64_t index = pq->size; index != ROOT; index = PARENT(index)) {
		uint64_t parent = PARENT(index);

		// Break loop if parent.time is smaller
		if(pq->events[index].time >= pq->events[parent].time) break;
		
		// Swap if parent.time is larger
		SWAP_EVENTS(pq, index, parent);
	}
}

es_event_t es_pq_dequeue(es_pq_t *pq) {
	// Can't dequeue if queue is empty
	assert(pq->size > 0);

	// The event at ROOT is the event with the 'highest' priority (lowest time value)
	es_event_t ev = pq->events[ROOT];

	// Replace ROOT with the event at the end of array (i.e. rightmost leaf)
	pq->events[ROOT] = pq->events[pq->size--];

	// Check if root.time is greater than its children, if so, swap with the smaller child
	// Do this until root.time is less than its children or root has no more children
	for(uint64_t index = ROOT, swap_index = ROOT; HAS_CHILDREN(index, pq);  index = swap_index) {
		uint64_t right = RIGHT(index), left = LEFT(index);

		// Check if root.time is greater than leftchild.time
		swap_index = ((pq->events[index].time >= pq->events[left].time) ? left : index);

		// Check if root.time or leftchild.time is greater than rightchild.time if the right child exists
		if(HAS_RIGHT_CHILD(index, pq)) {
			swap_index = ((pq->events[swap_index].time >= pq->events[right].time) ? right : swap_index);
		}

		// Break loop if the root.time is less than both of its children.time
		if(swap_index == index) break;

		// Swap root.time with the smaller of its two children
		SWAP_EVENTS(pq, index, swap_index);
	}
	
	// Check if size is <= half of capacity, if so, compact array
	//if(pq->size <= (pq->capacity>>1)) _es_pq_resize(pq, pq->capacity>>1);

	return ev;
}

es_event_t es_pq_at(es_pq_t* pq, uint64_t index) {
	assert(index <= pq->size && index > 0);
	return pq->events[index];
}

size_t es_pq_size(es_pq_t *pq) {
	return (pq->size);
}

void es_pq_delete(es_pq_t *pq) {
	// Free the event list
	free(pq->events);

	// Free the pq struct
	free(pq);
}

