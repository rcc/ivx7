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
 * matrix.h - Matrix and Vector Math
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
matrix_dt *new_matrix_dt(uint32_t rows, uint32_t cols);
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
