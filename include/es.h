#ifndef __ES_H__
#define __ES_H__

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
	TIMEOUT,
	ACK
} es_type_t;

typedef struct es_event {
	es_type_t event_type;
	double time;
  uint64_t rn; // ACK sequence number
  bool has_errors;
} es_event_t;

typedef struct es_pq es_pq_t;

// Returns a new pq
es_pq_t* es_pq_create(size_t capacity);

// Inserts the event into the pq, using the time property as the priority
void es_pq_enqueue(es_pq_t *pq, es_event_t ev);

// Dequeues the event with the highest priority
es_event_t es_pq_dequeue(es_pq_t *pq);

// Gets the event at index
es_event_t es_pq_at(es_pq_t *pq, uint64_t index);

// Returns the number of events in the pq
size_t es_pq_size(es_pq_t *pq);

// Deletes the pq
void es_pq_delete(es_pq_t *pq);

#endif // __ES_H__
