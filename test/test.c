#include <assert.h>
#include <stdio.h>

#include "doubles.h"
#include "vector.h"

int main(int argc, const char* argv[]) {
	int i;
  double d;

	printf("TESTING SETUP ...\n");
	Vector vector = VECTOR_INITIALIZER;
	assert(!vector_is_initialized(&vector));
	doubles_vector_setup(&vector, 0);
	assert(vector_is_initialized(&vector));

	printf("TESTING INSERT ...\n");

	for (i = 0; i < 1000; ++i) {
    d = (double)i;
		assert(vector_insert(&vector, 0, &d) == VECTOR_SUCCESS);
		assert(VECTOR_GET_AS(double, &vector, 0) == d);
		assert(vector_size(&vector) == i + 1);
	}

	double x = 5;
	vector_push_back(&vector, &x);
	vector_insert(&vector, vector_size(&vector), &x);

	printf("TESTING ASSIGNMENT ...\n");

	for (i = 0; i < vector_size(&vector); ++i) {
		double value = 666;
		assert(vector_assign(&vector, i, &value) == VECTOR_SUCCESS);
	}

	printf("TESTING ITERATION ...\n");

	Iterator iterator = vector_begin(&vector);
	assert(iterator_index(&vector, &iterator) == 0);

	iterator = vector_iterator(&vector, 1);
	assert(iterator_index(&vector, &iterator) == 1);

	VECTOR_FOR_EACH(&vector, iterator) {
		assert(ITERATOR_GET_AS(double, &iterator) == 666);
	}

	printf("TESTING REMOVAL ...\n");

	iterator = vector_begin(&vector);
	assert(iterator_erase(&vector, &iterator) == VECTOR_SUCCESS);

	size_t expected_size = vector_size(&vector);
	while (!vector_is_empty(&vector)) {
		assert(vector_erase(&vector, 0) == VECTOR_SUCCESS);
		assert(vector_size(&vector) == --expected_size);
	}

	printf("TESTING RESIZE ...\n");
	assert(vector_resize(&vector, 100) == VECTOR_SUCCESS);
	assert(vector_size(&vector) == 100);
	assert(vector_capacity(&vector) > 100);

	printf("TESTING RESERVE ...\n");
	assert(vector_reserve(&vector, 200) == VECTOR_SUCCESS);
	assert(vector_size(&vector) == 100);
	assert(vector_capacity(&vector) == 200);

	printf("TESTING CLEAR ...\n");
	assert(vector_clear(&vector) == VECTOR_SUCCESS);

	assert(vector_size(&vector) == 0);
	assert(vector_is_empty(&vector));
	assert(vector_capacity(&vector) == VECTOR_MINIMUM_CAPACITY);

	vector_destroy(&vector);

	printf("\033[92mALL TEST PASSED\033[0m\n");
}
