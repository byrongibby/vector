#include "doubles.h"
#include "vector.h"

int main(int argc, const char* argv[]) {
	Vector vector;
	double x, y, sum;

	/* Choose initial capacity of 10 */
  doubles_vector_setup(&vector, 10);

	x = 6.0, y = 9.0;
	vector_push_back(&vector, &x);
	vector_insert(&vector, 0, &y);
	vector_assign(&vector, 0, &y);

	x = *(double*)vector_get(&vector, 0);
	y = *(double*)vector_back(&vector);
	x = VECTOR_GET_AS(double, &vector, 1);

	vector_erase(&vector, 1);

	/* Iterator support */
	Iterator iterator = vector_begin(&vector);
	Iterator last = vector_end(&vector);
	for (; !iterator_equals(&iterator, &last); iterator_increment(&iterator)) {
		*(double*)iterator_get(&iterator) += 1;
	}

	/* Or just use pretty macros */
	sum = 0.0;
	VECTOR_FOR_EACH(&vector, i) {
		sum += ITERATOR_GET_AS(double, &i);
	}

	/* Memory management interface */
	vector_resize(&vector, 10);
	vector_reserve(&vector, 100);

	vector_clear(&vector);
	vector_destroy(&vector);
}
