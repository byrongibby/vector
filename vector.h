/* The MIT License (MIT)
 * Copyright (c) 2016 Peter Goldsborough
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

/***** DEFINITIONS *****/

#define VECTOR_MINIMUM_CAPACITY 2
#define VECTOR_GROWTH_FACTOR 2
#define VECTOR_SHRINK_THRESHOLD (1 / 4)

#define VECTOR_ERROR -1
#define VECTOR_SUCCESS 0

#define VECTOR_INITIALIZER { NULL, NULL }


/***** STRUCTURES *****/

typedef struct
{
  int (*const _iter_type)(void);
  void *(*const _iter_pointer)(void *self);
  void *(*const _iter_next)(void *self);
  void *(*const _iter_prev)(void *self);
} IteratorTC;

typedef struct {
  void *self;
  IteratorTC const *tc;
} Iterator;

typedef struct
{
  size_t (*const _vec_elem_size)(void);
  int (*const _vec_type)(void);
  size_t (*const _vec_size)(void *self);
  size_t (*const _vec_cap)(void *self);
  void *(*const _vec_data)(void *self);
  void (*const _vec_set_size)(void *self, size_t size);
  void (*const _vec_set_cap)(void *self, size_t capacity);
  void (*const _vec_set_data)(void *self, void *data);
  void *(*const _vec_offset)(void *self, size_t index);
  const void *(*const _vec_const_offset)(const void *self, size_t index);
  void *(*const _vec_offset_next)(void *offset);
  Iterator  (*const _vec_iterator)(void* self, size_t index);
  int (*const _vec_destroy)(void* self);
} VectorTC;

typedef struct
{
  void *self;
  VectorTC const *tc;
} Vector;


/***** METHODS *****/

/* Copy Constructor */
int vector_copy(Vector* destination, Vector* source);

/* Copy Assignment */
int vector_copy_assign(Vector* destination, Vector* source);

/* Move Constructor */
int vector_move(Vector* destination, Vector* source);

/* Move Assignment */
int vector_move_assign(Vector* destination, Vector* source);

int vector_swap(Vector* destination, Vector* source);

/* Destructor */
int vector_destroy(Vector* vector);

/* Insertion */
int vector_push_back(Vector* vector, void* element);
int vector_push_front(Vector* vector, void* element);
int vector_insert(Vector* vector, size_t index, void* element);
int vector_assign(Vector* vector, size_t index, void* element);

/* Deletion */
int vector_pop_back(Vector* vector);
int vector_pop_front(Vector* vector);
int vector_erase(Vector* vector, size_t index);
int vector_clear(Vector* vector);

/* Lookup */
void* vector_get(Vector* vector, size_t index);
const void* vector_const_get(const Vector* vector, size_t index);
void* vector_front(Vector* vector);
void* vector_back(Vector* vector);
#define VECTOR_GET_AS(type, vector_pointer, index) \
	*((type*)vector_get((vector_pointer), (index)))

/* Information */
bool vector_is_initialized(const Vector* vector);
size_t vector_byte_size(const Vector* vector);
size_t vector_size(const Vector* vector);
size_t vector_capacity(const Vector* vector);
size_t vector_free_space(const Vector* vector);
bool vector_is_empty(const Vector* vector);

/* Memory management */
int vector_resize(Vector* vector, size_t new_size);
int vector_reserve(Vector* vector, size_t minimum_capacity);
int vector_shrink_to_fit(Vector* vector);

/* Iterators */
Iterator vector_begin(Vector* vector);
Iterator vector_end(Vector* vector);
Iterator vector_iterator(Vector* vector, size_t index);

void* iterator_get(Iterator* iterator);
#define ITERATOR_GET_AS(type, iterator) *((type*)iterator_get((iterator)))

int iterator_erase(Vector* vector, Iterator* iterator);

void iterator_increment(Iterator* iterator);
void iterator_decrement(Iterator* iterator);

void* iterator_next(Iterator* iterator);
void* iterator_previous(Iterator* iterator);

bool iterator_equals(Iterator* first, Iterator* second);
bool iterator_is_before(Iterator* first, Iterator* second);
bool iterator_is_after(Iterator* first, Iterator* second);

size_t iterator_index(Vector* vector, Iterator* iterator);

#define VECTOR_FOR_EACH(vector_pointer, iterator_name)           \
	for (Iterator(iterator_name) = vector_begin((vector_pointer)), \
			end = vector_end((vector_pointer));                        \
			 !iterator_equals(&(iterator_name), &end);                 \
			 iterator_increment(&(iterator_name)))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif /* VECTOR_H */
