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
	
#define __STDC_WANT_LIB_EXT1__ 1

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

/***** PRIVATE *****/

bool _vec_should_grow(Vector *v)
{
	assert(v->tc->_vec_size(v->self) <= v->tc->_vec_cap(v->self));

	return v->tc->_vec_size(v->self) == v->tc->_vec_cap(v->self);
}

bool _vec_should_shrink(Vector *v)
{
	assert(v->tc->_vec_size(v->self) <= v->tc->_vec_cap(v->self));

	return v->tc->_vec_size(v->self) == v->tc->_vec_cap(v->self) *
    VECTOR_SHRINK_THRESHOLD;
}

size_t _vec_free_bytes(const Vector *v)
{
	return vector_free_space(v) * v->tc->_vec_elem_size();
}

void* _vec_offset(Vector *v, size_t index)
{
	return v->tc->_vec_offset(v->self, index);
}

const void* _vec_const_offset(const Vector *v, size_t index)
{
	return v->tc->_vec_const_offset(v->self, index);
}

void _vec_assign(Vector *v, size_t index, void* element)
{
	/* Insert the element */
	void* offset = _vec_offset(v, index);
	memcpy(offset, element, v->tc->_vec_elem_size());
}

int _vec_move_right(Vector *v, size_t index)
{
	assert(v->tc->_vec_size(v->self) < v->tc->_vec_cap(v->self));

	/* The location where to start to move from. */
	void* offset = _vec_offset(v, index);

	/* How many to move to the right. */
	size_t elements_in_bytes = (v->tc->_vec_size(v->self) - index) *
    v->tc->_vec_elem_size();

#ifdef __STDC_LIB_EXT1__
	size_t right_capacity_in_bytes = (v->tc->_vec_cap(v->self) - (index + 1)) *
      v->tc->_vec_elem_size();

	/* clang-format off */
  int return_code =  memmove_s(
      v->tc->_vec_offset_next(offset),
      right_capacity_in_bytes,
      offset,
      elements_in_bytes);
	/* clang-format on */

	return return_code == 0 ? VECTOR_SUCCESS : VECTOR_ERROR;

#else
	memmove(v->tc->_vec_offset_next(offset), offset, elements_in_bytes);
	return VECTOR_SUCCESS;
#endif
}

void _vec_move_left(Vector *v, size_t index)
{
	size_t right_elements_in_bytes;
	void* offset;

	/* The offset into the memory */
	offset = _vec_offset(v, index);

	/* How many to move to the left */
	right_elements_in_bytes = (v->tc->_vec_size(v->self) - index - 1) *
    v->tc->_vec_elem_size();

	memmove(offset, v->tc->_vec_offset_next(offset), right_elements_in_bytes);
}

int _vec_reallocate(Vector *v, size_t new_capacity)
{
	size_t new_capacity_in_bytes;
	void *data, *old;

	assert(v != NULL);
	assert(v->self != NULL);

	if (new_capacity < VECTOR_MINIMUM_CAPACITY) {
		if (v->tc->_vec_cap(v->self) > VECTOR_MINIMUM_CAPACITY) {
			new_capacity = VECTOR_MINIMUM_CAPACITY;
		} else {
			/* NO-OP */
			return VECTOR_SUCCESS;
		}
	}

	new_capacity_in_bytes = new_capacity * v->tc->_vec_elem_size();
	old = v->tc->_vec_data(v->self);

  data = malloc(new_capacity_in_bytes);
	if (data == NULL) return VECTOR_ERROR;

#ifdef __STDC_LIB_EXT1__
	/* clang-format off */
	if (memcpy_s(data,
        new_capacity_in_bytes,
        old,
        vector_byte_size(v)) != 0) {
		return VECTOR_ERROR;
	}
  /* clang-format on */
#else
	memcpy(data, old, vector_byte_size(v));
#endif

  v->tc->_vec_set_data(v->self, data);
  v->tc->_vec_set_cap(v->self, new_capacity);

	free(old);

	return VECTOR_SUCCESS;
}

int _vec_adjust_capacity(Vector *v)
{
	return _vec_reallocate(v,
      MAX(1, v->tc->_vec_size(v->self) * VECTOR_GROWTH_FACTOR));
}

int _vector_deinitialize(Vector *v)
{
	assert(v != NULL);
	assert(v->self != NULL);

	if (v == NULL) return VECTOR_ERROR;
	if (v->self == NULL) return VECTOR_ERROR;

	free(v->tc->_vec_data(v->self));
  v->tc->_vec_set_data(v->self, NULL);

	return VECTOR_SUCCESS;
}


/***** METHODS *****/

int vector_copy(Vector* dest, Vector* src)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(vector_is_initialized(src));
	assert(!vector_is_initialized(dest));
	assert(dest->tc->_vec_type() == src->tc->_vec_type());

	if (dest == NULL) return VECTOR_ERROR;
	if (src == NULL) return VECTOR_ERROR;
	if (vector_is_initialized(dest)) return VECTOR_ERROR;
	if (!vector_is_initialized(src)) return VECTOR_ERROR;
	if (dest->tc->_vec_type() != src->tc->_vec_type()) {
    return VECTOR_ERROR;
  }

	/* Copy ALL the data */
  dest->tc->_vec_set_size(dest->self, src->tc->_vec_size(src->self));
  dest->tc->_vec_set_cap(dest->self, dest->tc->_vec_size(dest->self) * 2);

	/* Note that we are not necessarily allocating the same capacity */
  void *data = malloc(dest->tc->_vec_cap(dest->self) *
      dest->tc->_vec_elem_size());
	if (data == NULL) return VECTOR_ERROR;

	memcpy(data, src->tc->_vec_data(src->self), vector_byte_size(src));

  dest->tc->_vec_set_data(dest->self, data);

	return VECTOR_SUCCESS;
}

int vector_copy_assign(Vector* dest, Vector* src)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(vector_is_initialized(src));
	assert(vector_is_initialized(dest));
	assert(dest->tc->_vec_type() == src->tc->_vec_type());

	if (dest == NULL) return VECTOR_ERROR;
	if (src == NULL) return VECTOR_ERROR;
	if (!vector_is_initialized(dest)) return VECTOR_ERROR;
	if (!vector_is_initialized(src)) return VECTOR_ERROR;
  if (dest->tc->_vec_type() != src->tc->_vec_type()) {
    return VECTOR_ERROR;
  }

	_vector_deinitialize(dest);

	return vector_copy(dest, src);
}

int vector_move(Vector* dest, Vector* src)
{
	assert(dest != NULL);
	assert(src != NULL);

	if (dest == NULL) return VECTOR_ERROR;
	if (src == NULL) return VECTOR_ERROR;

	*dest = *src;
  src->tc->_vec_set_data(src->self, NULL);

	return VECTOR_SUCCESS;
}

int vector_move_assign(Vector* dest, Vector* src)
{
	vector_swap(dest, src);
	return _vector_deinitialize(src);
}

int vector_swap(Vector* dest, Vector* src)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(vector_is_initialized(src));
	assert(vector_is_initialized(dest));
	assert(dest->tc->_vec_type() == src->tc->_vec_type());

	if (dest == NULL) return VECTOR_ERROR;
	if (src == NULL) return VECTOR_ERROR;
	if (!vector_is_initialized(dest)) return VECTOR_ERROR;
	if (!vector_is_initialized(src)) return VECTOR_ERROR;
	if (dest->tc->_vec_type() != src->tc->_vec_type()) {
    return VECTOR_ERROR;
  }

  size_t tmp_size = dest->tc->_vec_size(dest->self);
  dest->tc->_vec_set_size(dest->self, src->tc->_vec_size(src->self));
  src->tc->_vec_set_size(src->self, tmp_size);

  size_t tmp_capacity = dest->tc->_vec_cap(dest->self);
  dest->tc->_vec_set_cap(dest->self, src->tc->_vec_cap(src->self));
  src->tc->_vec_set_cap(src->self, tmp_capacity);

	void *tmp_data = dest->tc->_vec_data(dest->self);
	dest->tc->_vec_set_data(dest->self, src->tc->_vec_data(src->self));
	src->tc->_vec_set_data(src->self, tmp_data);

	return VECTOR_SUCCESS;
}

int vector_destroy(Vector *v)
{
	assert(v != NULL);

	if (v == NULL) return VECTOR_ERROR;

  if (v->tc->_vec_destroy(v->self) == 0) {
    v->self = NULL;
    return VECTOR_SUCCESS;
  }

  return VECTOR_ERROR;
}

/* Insertion */

int vector_push_back(Vector *v, void* element)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(element != NULL);

	if (_vec_should_grow(v)) {
		if (_vec_adjust_capacity(v) == VECTOR_ERROR) {
			return VECTOR_ERROR;
		}
	}

	_vec_assign(v, v->tc->_vec_size(v->self), element);

  v->tc->_vec_set_size(v->self, v->tc->_vec_size(v->self) + 1);

	return VECTOR_SUCCESS;
}

int vector_push_front(Vector *v, void* element)
{
	return vector_insert(v, 0, element);
}

int vector_insert(Vector *v, size_t index, void* element)
{
	void* offset;

	assert(v != NULL);
	assert(v->self != NULL);
	assert(element != NULL);
	assert(index <= v->tc->_vec_size(v->self));

	if (v == NULL) return VECTOR_ERROR;
	if (v->self == NULL) return VECTOR_ERROR;
	if (element == NULL) return VECTOR_ERROR;
	if (index > v->tc->_vec_size(v->self)) return VECTOR_ERROR;

  if (_vec_should_grow(v)) {
    if (_vec_adjust_capacity(v) == VECTOR_ERROR) {
      return VECTOR_ERROR;
    }
  }

	/* Move other elements to the right */
	if (_vec_move_right(v, index) == VECTOR_ERROR) {
		return VECTOR_ERROR;
	}

	/* Insert the element */
	offset = _vec_offset(v, index);
	memcpy(offset, element, v->tc->_vec_elem_size());
  v->tc->_vec_set_size(v->self, v->tc->_vec_size(v->self) + 1);

	return VECTOR_SUCCESS;
}

int vector_assign(Vector *v, size_t index, void* element)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(element != NULL);
	assert(index < v->tc->_vec_size(v->self));

	if (v == NULL) return VECTOR_ERROR;
	if (v->self == NULL) return VECTOR_ERROR;
	if (element == NULL) return VECTOR_ERROR;
	if (index >= v->tc->_vec_size(v->self)) return VECTOR_ERROR;

	_vec_assign(v, index, element);

	return VECTOR_SUCCESS;
}

/* Deletion */

int vector_pop_back(Vector *v)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(v->tc->_vec_size(v->self) > 0);

	if (v == NULL) return VECTOR_ERROR;
	if (v->self == NULL) return VECTOR_ERROR;

  v->tc->_vec_set_size(v->self, v->tc->_vec_size(v->self) - 1);

#ifndef VECTOR_NO_SHRINK
	if (_vec_should_shrink(v)) {
		_vec_adjust_capacity(v);
	}
#endif

	return VECTOR_SUCCESS;
}

int vector_pop_front(Vector *v)
{
	return vector_erase(v, 0);
}

int vector_erase(Vector *v, size_t index)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(index < v->tc->_vec_size(v->self));

	if (v == NULL) return VECTOR_ERROR;
	if (v->self == NULL) return VECTOR_ERROR;
	if (index >= v->tc->_vec_size(v->self)) return VECTOR_ERROR;

	/* Just overwrite */
	_vec_move_left(v, index);

#ifndef VECTOR_NO_SHRINK
  v->tc->_vec_set_size(v->self, v->tc->_vec_size(v->self) - 1);
	if (v->tc->_vec_size(v->self) == v->tc->_vec_cap(v->self) / 4) {
		_vec_adjust_capacity(v);
	}
#endif

	return VECTOR_SUCCESS;
}

int vector_clear(Vector *v)
{
	return vector_resize(v, 0);
}

/* Lookup */

void* vector_get(Vector *v, size_t index)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(index < v->tc->_vec_size(v->self));

	if (v == NULL) return NULL;
	if (v->self == NULL) return NULL;
	if (index >= v->tc->_vec_size(v->self)) return NULL;

	return _vec_offset(v, index);
}

const void* vector_const_get(const Vector *v, size_t index)
{
  assert(v != NULL);
	assert(v->self != NULL);
  assert(index < v->tc->_vec_size(v->self));

  if (v == NULL) return NULL;
	if (v->self == NULL) return NULL;
  if (index >= v->tc->_vec_size(v->self)) return NULL;

  return _vec_const_offset(v, index);
}

void* vector_front(Vector *v)
{
	return vector_get(v, 0);
}

void* vector_back(Vector *v)
{
	return vector_get(v, v->tc->_vec_size(v->self) - 1);
}

/* Information */

bool vector_is_initialized(const Vector *v)
{
	return v->self != NULL && v->tc->_vec_data(v->self) != NULL;
}

size_t vector_byte_size(const Vector *v)
{
	assert(v->self != NULL);
	return v->tc->_vec_size(v->self) * v->tc->_vec_elem_size();
}

size_t vector_size(const Vector *v)
{
	assert(v->self != NULL);
	return v->tc->_vec_size(v->self);
}

size_t vector_capacity(const Vector *v)
{
	assert(v->self != NULL);
	return v->tc->_vec_cap(v->self);
}

size_t vector_free_space(const Vector *v)
{
	assert(v->self != NULL);
	return v->tc->_vec_cap(v->self) - v->tc->_vec_size(v->self);
}

bool vector_is_empty(const Vector *v)
{
	assert(v->self != NULL);
	return v->tc->_vec_size(v->self) == 0;
}

/* Memory management */

int vector_resize(Vector *v, size_t new_size)
{
	if (new_size <= v->tc->_vec_cap(v->self) * VECTOR_SHRINK_THRESHOLD) {
    v->tc->_vec_set_size(v->self, new_size);
		if (_vec_reallocate(v, new_size * VECTOR_GROWTH_FACTOR) == -1) {
			return VECTOR_ERROR;
		}
	} else if (new_size > v->tc->_vec_cap(v->self)) {
		if (_vec_reallocate(v, new_size * VECTOR_GROWTH_FACTOR) == -1) {
			return VECTOR_ERROR;
		}
	}

  v->tc->_vec_set_size(v->self, new_size);

	return VECTOR_SUCCESS;
}

int vector_reserve(Vector *v, size_t minimum_capacity)
{
	if (minimum_capacity > v->tc->_vec_cap(v->self)) {
		if (_vec_reallocate(v, minimum_capacity) == VECTOR_ERROR) {
			return VECTOR_ERROR;
		}
	}

	return VECTOR_SUCCESS;
}

int vector_shrink_to_fit(Vector *v)
{
	return _vec_reallocate(v, v->tc->_vec_size(v->self));
}

/* Iterators */

Iterator vector_begin(Vector *v)
{
	return vector_iterator(v, 0);
}

Iterator vector_end(Vector *v)
{
	return vector_iterator(v, v->tc->_vec_size(v->self));
}

Iterator vector_iterator(Vector *v, size_t index)
{
	return v->tc->_vec_iterator(v->self, index);
}

void* iterator_get(Iterator* iter)
{
	return iter->tc->_iter_pointer(iter->self);
}

int iterator_erase(Vector *v, Iterator *iter)
{
	size_t index = iterator_index(v, iter);

	if (vector_erase(v, index) == VECTOR_ERROR) {
		return VECTOR_ERROR;
	}

	*iter = vector_iterator(v, index);

	return VECTOR_SUCCESS;
}

void iterator_increment(Iterator* iter)
{
	assert(iter != NULL);
  iter->self = iter->tc->_iter_next(iter->self);
}

void iterator_decrement(Iterator* iter)
{
	assert(iter != NULL);
  iter->self = iter->tc->_iter_prev(iter->self);
}

void* iterator_next(Iterator* iter)
{
	void* current = iterator_get(iter);
	iterator_increment(iter);

	return current;
}

void* iterator_previous(Iterator* iter)
{
	void* current = iterator_get(iter);
	iterator_decrement(iter);

	return current;
}

bool iterator_equals(Iterator* first, Iterator* second)
{
	assert(first->tc->_iter_type() ==  second->tc->_iter_type());

	return iterator_get(first) == iterator_get(second);
}

bool iterator_is_before(Iterator* first, Iterator* second)
{
  assert(first->tc->_iter_type() == second->tc->_iter_type());

	return iterator_get(first) < iterator_get(second);
}

bool iterator_is_after(Iterator* first, Iterator* second)
{
	assert(first->tc->_iter_type() == second->tc->_iter_type());

	return iterator_get(first) > iterator_get(second);
}

size_t iterator_index(Vector *v, Iterator* iter)
{
	assert(v != NULL);
	assert(v->self != NULL);
	assert(iter != NULL);
	assert(v->tc->_vec_type() == iter->tc->_iter_type());

	return (iter->tc->_iter_pointer(iter->self) - v->tc->_vec_data(v->self)) /
    v->tc->_vec_elem_size();
}
