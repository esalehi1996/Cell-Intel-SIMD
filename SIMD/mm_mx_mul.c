/* SIMD Matrix Multiplication */
/* compile with `gcc -msse4.1 mm_mx_mul.c` */

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

float** create_matrix(int n, int m, int do_init) {
    float **a = (float**) malloc(n * sizeof(float*));
    for (int i = 0; i < n; i += 1) {
        a[i] = (float*) malloc(n * sizeof(float));
        if (do_init)
            memset(a[i], 0, m * sizeof(float));
    }
    return a;
}

void fill_random(float **a, int n, int m) {
    for (int i = 0; i < n; i += 1)
        for (int j = 0; j < n; j += 1)
            a[i][j] = 1.0;
}

void read_matrix(float **a, int n, int m, int do_transpose) {
    for (int i = 0; i < n; i += 1)
        for (int j = 0; j < m; j += 1)
            scanf("%f", (do_transpose ? &a[j][i] : &a[i][j]));
}

void write_matrix(float **a, int n, int m) {
    for (int i = 0; i < n; i += 1) {
        for (int j = 0; j < m; j += 1)
            printf("%6g ", a[i][j]);
        printf("\n");
    }
}

/* returns 0 if matrices are equal, 1 otherwise */
int compare_matrix(float **a, float **b, int n, int m) {
    static const float EPS = 1E-2;
    for (int i = 0; i < n; i += 1)
        for (int j = 0; j < n; j += 1)
            if (abs(a[i][j] - b[i][j]) > EPS)
                return 1;
    return 0;
}

/* b is transposed */
void mm_mx_mul_4x4(float **a, int ai, int aj, float **b, int bi, int bj, float **c, int ci, int cj) {
    __m128 u, v;
    float x[4];
    for (int i = 0; i < 4; i += 1)
        for (int j = 0; j < 4; j += 1) {
            u = _mm_load_ps(&a[ai + i][aj]);
            v = _mm_load_ps(&b[bi + j][bj]);
            u = _mm_dp_ps(u, v, 0xFF);
            _mm_store_ps(x, u);
            c[ci + i][cj + j] += x[0];
        }
}

/* b is transposed */
void mm_mx_mul(float **a, int an, int am, float **b, int bn, int bm, float **c) {
    assert(am == bm);
    for (int i = 0; i < an; i += 4)
        for (int j = 0; j < bn; j += 4)
            for (int k = 0; k < am; k += 4)
                mm_mx_mul_4x4(a, i, k, b, j, k, c, i, j);
}

/* b is transposed */
void mx_mul(float **a, int an, int am, float **b, int bn, int bm, float **c) {
    assert(am == bm);
    for (int i = 0; i < an; i += 1)
        for (int j = 0; j < bn; j += 1)
            for (int k = 0; k < am; k += 1)
                c[i][j] += a[i][k] * b[j][k];
}

/* when `IO == 1` program will receive input from stdin and write result to stdout */
#define IO 0

int main() {
    int n;
    
    if (IO) {
        scanf("%d", &n);
    } else {
        n = 1000;
    }

    /* pad n to multiple of 4 */
    int n4 = n + (4 - n % 4) % 4;
    float **a = create_matrix(n4, n4, 1);
    float **b = create_matrix(n4, n4, 1);
    float **c = create_matrix(n4, n4, 1);
    float **gold = create_matrix(n4, n4, 1);
    
    if (IO) {
        read_matrix(a, n, n, 0);
        read_matrix(b, n, n, 1);
    } else {
        fill_random(a, n, n);
        fill_random(b, n, n);
    }
    
    time_t start, stop;

    start = clock();
    mx_mul(a, n, n, b, n, n, gold);
    stop = clock();
    fprintf(stderr, "   [mx_mul] Time: %g ms\n", (stop - start) / (double)(CLOCKS_PER_SEC / 1000));
    
    start = clock();
    mm_mx_mul(a, n, n, b, n, n, c);
    stop = clock();
    fprintf(stderr, "[mm_mx_mul] Time: %g ms\n", (stop - start) / (double)(CLOCKS_PER_SEC / 1000));

    int ok = compare_matrix(c, gold, n, n);
    if (ok == 0) fprintf(stderr, "Outputs match. Success!\n");
    else fprintf(stderr, "Outputs don't match. Failed!\n");

    
        write_matrix(c, n, n);
    

    return 0;
}
