#define _GNU_SOURCE
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* ---------- helpers ---------------------------------------------------- */

static inline uint64_t now_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void rand_matrix(int* m, int n) {
	for (int i = 0; i < n * n; ++i) m[i] = rand() % 10; /* 0 … 9 */
}

static int verify(int* c, int* ref, int n) { return memcmp(c, ref, sizeof(int) * n * n) == 0; }

/* ---------- sequential ------------------------------------------------- */

static void mul_seq(int const* a, int const* b, int* c, int n) {
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j) {
			int sum = 0;
			for (int k = 0; k < n; ++k) sum += a[i * n + k] * b[k * n + j];
			c[i * n + j] = sum;
		}
}

/* ---------- parallel helpers ------------------------------------------ */

struct task {
	int const* a;
	int const* b;
	int* c;
	int n;
	int row_start;
	int row_end; /* exclusive */
};

static void* mul_rows(void* arg) {
	struct task* t = (struct task*)arg;
	int const *a = t->a, *b = t->b;
	int *c = t->c, n = t->n;

	for (int i = t->row_start; i < t->row_end; ++i)
		for (int j = 0; j < n; ++j) {
			int sum = 0;
			for (int k = 0; k < n; ++k) sum += a[i * n + k] * b[k * n + j];
			c[i * n + j] = sum;
		}
	return NULL;
}

static uint64_t mul_parallel(int const* a, int const* b, int* c, int n, int threads) {
	pthread_t* tid = malloc(sizeof(pthread_t) * threads);
	struct task* tsk = malloc(sizeof(struct task) * threads);

	int rows_per = (n + threads - 1) / threads;
	int row = 0;
	uint64_t t0 = now_ns();

	for (int t = 0; t < threads; ++t) {
		int start = row;
		int end = MIN(n, start + rows_per);
		tsk[t] = (struct task){a, b, c, n, start, end};
		pthread_create(&tid[t], NULL, mul_rows, &tsk[t]);
		row = end;
	}
	for (int t = 0; t < threads; ++t) pthread_join(tid[t], NULL);

	uint64_t t1 = now_ns();
	free(tid);
	free(tsk);
	return t1 - t0;
}

/* ---------- main driver ----------------------------------------------- */

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "usage: %s <n> <runs>\n", argv[0]);
		return 1;
	}
	int n = atoi(argv[1]);
	int runs = atoi(argv[2]);
	if (n <= 0 || runs <= 0) {
		fputs("n and runs must be positive\n", stderr);
		return 1;
	}

	srand((unsigned)time(NULL));

	int* A = malloc(sizeof(int) * n * n);
	int* B = malloc(sizeof(int) * n * n);
	int* C1 = malloc(sizeof(int) * n * n);
	int* C2 = malloc(sizeof(int) * n * n);
	int* Cn = malloc(sizeof(int) * n * n);

	if (!A || !B || !C1 || !C2 || !Cn) {
		fputs("malloc failed\n", stderr);
		return 1;
	}

	for (int r = 0; r < runs; ++r) {
		rand_matrix(A, n);
		rand_matrix(B, n);

		uint64_t t1, t2, tn;

		/* version 1: sequential */
		uint64_t s0 = now_ns();
		mul_seq(A, B, C1, n);
		t1 = now_ns() - s0;

		/* version 2: two threads */
		t2 = mul_parallel(A, B, C2, n, 2);
		if (!verify(C2, C1, n)) {
			fputs("ERROR v2\n", stderr);
			return 1;
		}

		/* version n: one thread per row */
		tn = mul_parallel(A, B, Cn, n, n);
		if (!verify(Cn, C1, n)) {
			fputs("ERROR vn\n", stderr);
			return 1;
		}

		printf("%" PRIu64 " %" PRIu64 " %" PRIu64 "\n", t1, t2, tn);
		fflush(stdout);
	}

	free(A);
	free(B);
	free(C1);
	free(C2);
	free(Cn);
	return 0;
}
