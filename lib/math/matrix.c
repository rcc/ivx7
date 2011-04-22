/*
 * Copyright 2011 Robert C. Curtis. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBERT C. CURTIS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ROBERT C. CURTIS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Robert C. Curtis.
 */

/*
 * matrix.c - Matrix and Vector Math Library
 */

#include <matrix.h>
#include <stdlib.h>
#include <string.h>

/***************************** Allocate and Free ****************************/
matrix_dt *new_matrix_dt(uint32_t rows, uint32_t cols)
{
	matrix_dt *m = malloc(sizeof(matrix_dt) +
			(sizeof(m->values[0]) * cols * rows));

	if(m) {
		m->cols = cols;
		m->rows = rows;
	}

	return m;
}

void free_matrix_dt(matrix_dt *m)
{
	free(m);
}

matrix_dt *copy_matrix_dt(const matrix_dt *A)
{
	matrix_dt *m;

	if(!(m = new_matrix_dt(A->rows, A->cols)))
		return NULL;

	memcpy(m->values, A->values,
			sizeof(m->values[0]) * m->cols * m->rows);

	return m;
}

void zero_matrix_dt(matrix_dt *A)
{
	memset(A->values, 0, sizeof(A->values[0]) * A->cols * A->rows);
}

/********************************* Utilities ********************************/
#if 0
static uint32_t factorial(uint32_t n)
{
	uint32_t i, f = 1;

	for(i = 2; i <= n; i++)
		f *= i;

	return f;
}
#endif

/* returns 0 when the permatation is already the final permutation,
 * 1 otherwise
 */
static int permute_array(uint32_t *a, size_t count)
{
	/* The following algorithm generates the next permutation
	 * lexicographically after a given permutation. It changes the given
	 * permutation in-place.
	 * 1. Find the largest index k such that a[k] < a[k + 1]. If no such
	 * 	index exists, the permutation is the last permutation.
	 * 2. Find the largest index l such that a[k] < a[l]. Since k + 1 is
	 * 	such an index, l is well defined and satisfies k < l.
	 * 3. Swap a[k] with a[l].
	 * 4. Reverse the sequence from a[k + 1] up to and including the final
	 * 	element a[n].
	 */
	size_t k = (count - 2), i = (count - 1);
	uint32_t s;

	if(count < 2)
		return 0;

	while(k > 0) {
		if(a[k] < a[k + 1])
			break;
		k--;
	}
	if(a[k] >= a[k + 1])
		return 0;

	while(i > (k + 1)) {
		if(a[k] < a[i])
			break;
		i--;
	}
	s = a[k];
	a[k] = a[i];
	a[i] = s;

	for(k += 1, i = (count - 1); k < i; k++, i--) {
		s = a[k];
		a[k] = a[i];
		a[i] = s;
	}

	return 1;
}

static unsigned int array_inversions(uint32_t *a, size_t count)
{
	size_t i, j;
	unsigned int inversions = 0;

	for(i = 0; i < count; i++) {
		for(j = i + 1; j < count; j++) {
			if(a[i] > a[j])
				inversions++;
		}
	}

	return inversions;
}

/********************************* Addition *********************************/
matrix_dt *add_matrix_dt(const matrix_dt *A, const matrix_dt *B)
{
	matrix_dt *m;
	uint32_t i;

	if((A->cols != B->cols) || (A->rows != B->rows))
		return NULL;

	if(!(m = new_matrix_dt(A->rows, A->cols)))
		return NULL;

	for(i = 0; i < (m->cols * m->rows); i++) {
		m->values[i] = A->values[i] + B->values[i];
	}

	return m;
}

/*************************** Scalar Multiplication **************************/
matrix_dt *scalar_mult_matrix_dt(const matrix_dt *A, double S)
{
	matrix_dt *m;
	uint32_t i;

	if(!(m = new_matrix_dt(A->rows, A->cols)))
		return NULL;

	for(i = 0; i < (m->cols * m->rows); i++) {
		m->values[i] = A->values[i] * S;
	}

	return m;
}

/********************************* Transpose ********************************/
matrix_dt *transpose_matrix_dt(const matrix_dt *A)
{
	matrix_dt *m;
	uint32_t i,j;

	if(!(m = new_matrix_dt(A->cols, A->rows)))
		return NULL;

	for(i = 0; i < A->rows; i++) {
		for(j = 0; j < A->cols; j++) {
			MATRIX_VAL(m, j, i) = MATRIX_VAL(A, i, j);
		}
	}

	return m;
}

/*************************** Matrix Multiplication **************************/
matrix_dt *mult_matrix_dt(const matrix_dt *A, const matrix_dt *B)
{
	matrix_dt *m;
	uint32_t i,j,k;

	if(A->cols != B->rows)
		return NULL;

	if(!(m = new_matrix_dt(A->rows, B->cols)))
		return NULL;

	zero_matrix_dt(m);

	for(i = 0; i < A->rows; i++) {
		for(j = 0; j < B->cols; j++) {
			for(k = 0; k < A->cols; k++) {
				m->values[i * m->cols + j] +=
					A->values[i * A->cols + k] *
					B->values[k * B->cols + j];
			}
		}
	}

	return m;
}

/******************************** Determinant *******************************/
double determinant_dt(const matrix_dt *A)
{
	/* Brute force determinant calculation. Nothing elegant (or fast) about
	 * it.
	 */
	double det = 0.0;
	uint32_t *p;
	uint32_t i;

	if(A->rows != A->cols)
		return 0;

	if(!(p = malloc(sizeof(p[0]) * A->cols)))
		return 0;

	for(i = 0; i < A->cols; i++)
		p[i] = i;

	do {
		double product;

		if(array_inversions(p, A->cols) & 0x1) {
			/* odd - negative */
			product = -1.0;
		} else {
			/* even - positive */
			product = 1.0;
		}

		for(i = 0; i < A->cols; i++) {
			product *= A->values[p[i] * A->cols + i];
		}

		det += product;

	} while(permute_array(p, A->rows));

	free(p);
	return det;
}

/********************************* Cofactor *********************************/
/* k(i,j) = (-1)^(i + j) * det(M(i,j))
 * where M(i,j) is the (n-1)x(n-1) matrix obtained by deleting row i and
 * column j from the matrix A.
 */
double cofactor_dt(const matrix_dt *A, uint32_t i, uint32_t j)
{
	double c;
	matrix_dt *m;
	uint32_t k, l;

	if(!(m = new_matrix_dt(A->rows - 1, A->cols - 1)))
		return 0;

	for(k = 0, l = 0; k < (A->cols * A->rows); k++) {
		if(((k / A->cols) == i) || ((k % A->cols) == j))
			continue;
		m->values[l++] = A->values[k];
	}

	c = determinant_dt(m);
	if((i + j) & 0x1)
		c *= -1.0;

	free_matrix_dt(m);

	return c;
}

/****************************** Cofactor Matrix *****************************/
matrix_dt *cofactor_matrix_dt(const matrix_dt *A)
{
	matrix_dt *m;
	uint32_t i, j;

	if(!(m = new_matrix_dt(A->rows, A->cols)))
		return NULL;

	for(i = 0; i < A->rows; i++) {
		for(j = 0; j < A->cols; j++) {
			MATRIX_VAL(m, i, j) = cofactor_dt(A, i, j);
		}
	}

	return m;
}

/********************************** Inverse *********************************/
matrix_dt *inverse_matrix_dt(const matrix_dt *A)
{
	matrix_dt *m, *Ac, *Act;
	double det = determinant_dt(A);

	/* Can only invert a matrix with a non-zero determinant */
	if(det == 0.0)
		return NULL;

	if(!(Ac = cofactor_matrix_dt(A))) {
		return NULL;
	}

	if(!(Act = transpose_matrix_dt(Ac))) {
		free_matrix_dt(Ac);
		return NULL;
	}

	m = scalar_mult_matrix_dt(Act, 1 / det);

	free_matrix_dt(Ac);
	free_matrix_dt(Act);
	return m;
}
