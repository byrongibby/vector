#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "doubles.h"


/***** ITERATOR *****/

int doubles_iter_type(void)
{
  return 1;
}

double *doubles_iter_pointer(double *pointer)
{
	assert(pointer != NULL);
  return pointer;
}

double *doubles_iter_next(double *pointer)
{
	assert(pointer != NULL);
	return pointer + 1;
}

double *doubles_iter_prev(double *pointer)
{
	assert(pointer != NULL);
	return pointer - 1;
}

/* Wrapper functions */

static inline int doubles_iter_type__(void)
{
  return doubles_iter_type();
}

static inline void *doubles_iter_pointer__(void *self)
{
  return doubles_iter_pointer(self);
}

static inline void *doubles_iter_next__(void *self)
{
  return doubles_iter_next(self);
}

static inline void *doubles_iter_prev__(void *self)
{
  return doubles_iter_prev(self);
}


/***** VECTOR *****/

int doubles_setup(Doubles *doubles, size_t capacity)
{
	assert(doubles != NULL);

	if (doubles == NULL) return VECTOR_ERROR;

	doubles->size = 0;
	doubles->capacity = MAX(VECTOR_MINIMUM_CAPACITY, capacity);
	doubles->data = malloc(doubles->capacity * sizeof(double));

	return doubles->data == NULL ? VECTOR_ERROR : VECTOR_SUCCESS;
}

size_t doubles_elem_size(void)
{
  return sizeof(double);
}

int doubles_type(void)
{
  return 1;
}

size_t doubles_size(Doubles *doubles)
{
	assert(doubles != NULL);
  return doubles->size;
}

size_t doubles_capacity(Doubles *doubles)
{
	assert(doubles != NULL);
  return doubles->capacity;
}

double *doubles_data(Doubles *doubles)
{
	assert(doubles != NULL);
  return doubles->data;
}

void doubles_set_size(Doubles *doubles, size_t size)
{
	assert(doubles != NULL);
  doubles->size = size;
}

void doubles_set_capacity(Doubles *doubles, size_t capacity)
{
	assert(doubles != NULL);
  doubles->capacity = capacity;
}

void doubles_set_data(Doubles *doubles, double *data)
{
	assert(doubles != NULL);
  doubles->data = data;
}

double *doubles_offset(Doubles *doubles, size_t index)
{
	assert(doubles != NULL);
  return doubles->data + index;
}

const double *doubles_const_offset(const Doubles *doubles, size_t index)
{
	assert(doubles != NULL);
  return doubles->data + index;
}

double *doubles_offset_next(double *offset)
{
	assert(offset != NULL);
  return offset + 1;
}

Iterator doubles_iterator(Doubles *doubles, size_t index) {
  Iterator iterator = { NULL, NULL };

  assert(doubles != NULL);
  assert(index <= doubles->size);

  if (doubles == NULL) return iterator;
  if (index > doubles->size) return iterator;

  static IteratorTC const iterator_tc = {
    ._iter_type    = doubles_iter_type__,
    ._iter_pointer = doubles_iter_pointer__,
    ._iter_next    = doubles_iter_next__,
    ._iter_prev    = doubles_iter_prev__,
  };

  iterator.tc = &iterator_tc;
  iterator.self = doubles_offset(doubles, index);

  return iterator;
}

int doubles_destroy(Doubles *doubles) {
	assert(doubles != NULL);

	if (doubles == NULL) return VECTOR_ERROR;

	if (doubles->data != NULL) free(doubles->data);
  free(doubles);

	return VECTOR_SUCCESS;
}

/* Wrapper functions */

static inline size_t doubles_elem_size__(void)
{
  return doubles_elem_size();
}

static inline int doubles_type__(void)
{
  return doubles_type();
}

static inline size_t doubles_size__(void *self)
{
  return doubles_size(self);
}

static inline size_t doubles_capacity__(void *self)
{
  return doubles_capacity(self);
}

static inline void *doubles_data__(void *self)
{
  return doubles_data(self);
}

static inline void doubles_set_size__(void *self, size_t size)
{
  doubles_set_size(self, size);
}

static inline void doubles_set_capacity__(void *self, size_t capacity)
{
  doubles_set_capacity(self, capacity);
}

static inline void doubles_set_data__(void *self, void *data)
{
  doubles_set_data(self, data);
}

static inline void *doubles_offset__(void *self, size_t index)
{
  return doubles_offset(self, index);
}

static inline const void *doubles_const_offset__(const void *self, size_t index)
{
  return doubles_const_offset(self, index);
}

static inline void *doubles_offset_next__(void *offset)
{
  return doubles_offset_next(offset);
}

static inline Iterator doubles_iterator__(void *self, size_t index)
{
  return doubles_iterator(self, index);
}

static inline int doubles_destroy__(void *self)
{
  return doubles_destroy(self);
}

/* Make function to build a generic `Vector` out of a concrete type - `Doubles` */

int doubles_vector_setup(Vector *vector, size_t capacity)
{
	assert(vector != NULL);

	if (vector == NULL) return VECTOR_ERROR;

  Doubles *doubles = malloc(sizeof(Doubles));

  /* Build the vtable once and attach a pointer to it every time */
  static VectorTC const vector_tc = {
    ._vec_elem_size    = doubles_elem_size__,
    ._vec_type         = doubles_type__,
    ._vec_size         = doubles_size__,
    ._vec_cap          = doubles_capacity__,
    ._vec_data         = doubles_data__,
    ._vec_set_size     = doubles_set_size__,
    ._vec_set_cap      = doubles_set_capacity__,
    ._vec_set_data     = doubles_set_data__,
    ._vec_offset       = doubles_offset__,
    ._vec_const_offset = doubles_const_offset__,
    ._vec_offset_next  = doubles_offset_next__,
    ._vec_iterator     = doubles_iterator__,
    ._vec_destroy      = doubles_destroy__,
  };

  if (doubles_setup(doubles, capacity) == VECTOR_ERROR) {
    return VECTOR_ERROR;
  }

  vector->tc = &vector_tc;
  vector->self = doubles;

  return VECTOR_SUCCESS;
}
