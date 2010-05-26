/*
 * matrix.h - Matrix and Vector Math
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * matrix.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * matrix.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with matrix.h.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>

#ifndef I__MATRIX_H__
	#define I__MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Matrix of doubles */
typedef struct matrix_dt {
	uint32_t cols;
	uint32_t rows;
	double values[];
} matrix_dt;

#define MATRIX_VAL(mp, row, col) ((mp)->values[(row) * (mp)->cols + (col)])

/* Allocate and Free */
matrix_dt *new_matrix_dt(uint32_t cols, uint32_t rows);
void free_matrix_dt(matrix_dt *m);
matrix_dt *copy_matrix_dt(const matrix_dt *A);
void zero_matrix_dt(matrix_dt *A);

/* Addition */
matrix_dt *add_matrix_dt(const matrix_dt *A, const matrix_dt *B);

/* Scalar Multiplication */
matrix_dt *scalar_mult_matrix_dt(const matrix_dt *A, double S);

/* Transpose */
matrix_dt *transpose_matrix_dt(const matrix_dt *A);

/* Matrix Multiplication */
matrix_dt *mult_matrix_dt(const matrix_dt *A, const matrix_dt *B);

/* Determinant */
double determinant_dt(const matrix_dt *A);

/* Cofactor */
double cofactor_dt(const matrix_dt *A, uint32_t i, uint32_t j);

/* Cofactor Matrix */
matrix_dt *cofactor_matrix_dt(const matrix_dt *A);

/* Matrix Inversion */
matrix_dt *inverse_matrix_dt(const matrix_dt *A);

#ifdef __cplusplus
}
#endif
#endif /* I__MATRIX_H__ */
