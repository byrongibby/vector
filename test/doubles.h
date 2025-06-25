#ifndef DOUBLES_H
#define DOUBLES_H

#include <stdlib.h>

#include "vector.h"

typedef struct Doubles {
	size_t size;
	size_t capacity;
	double *data;
} Doubles;

int doubles_vector_setup(Vector *vector, size_t capacity);

#endif /* DOUBLES_H */
