#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

/** Computes determinant of 3x3 matrix @numbers,
*   ARM assembler version
*/
int32_t count_det(int32_t *numbers);

#define N 3
#define ANS ((N) * (N))
#define IDX(I, J) ((N) * (I) + (J))

/** Computes determinant of 3x3 matrix @numbers,
*   C-version for unit tests
*/
int32_t count_det_test(int32_t *numbers) {
  int32_t det = 0;
  det += numbers[IDX(0, 0)] * numbers[IDX(1, 1)] * numbers[IDX(2, 2)];
  det += numbers[IDX(0, 1)] * numbers[IDX(1, 2)] * numbers[IDX(2, 0)];
  det += numbers[IDX(0, 2)] * numbers[IDX(1, 0)] * numbers[IDX(2, 1)];
  det -= numbers[IDX(0, 0)] * numbers[IDX(1, 2)] * numbers[IDX(2, 1)];
  det -= numbers[IDX(0, 1)] * numbers[IDX(1, 0)] * numbers[IDX(2, 2)];
  det -= numbers[IDX(0, 2)] * numbers[IDX(1, 1)] * numbers[IDX(2, 0)];
  return det;
}

void run_tests() {
  {
    int32_t numbers[N * N + 1] = {
      1, 2, 3,
      4, 5, 6,
      7, 8, 9,
      0
    };
    int32_t r0 = numbers[ANS];
    int32_t r1 = count_det_test(numbers);
    int32_t r2 = count_det(numbers);
    if (r0 != r1 || r0 != r2) {
      printf("Error: r0 = %d, r1 = %d, r2 = %d\n", r0, r1, r2);
      assert(0);
    }
  }

  {
    int32_t numbers[N * N + 1] = {
       1,  2,  -3,
       4,  5,   6,
      -7,  8,   9,
      -360
    };
    int32_t r0 = numbers[ANS];
    int32_t r1 = count_det_test(numbers);
    int32_t r2 = count_det(numbers);
    if (r0 != r1 || r0 != r2) {
      printf("Error: r0 = %d, r1 = %d, r2 = %d\n", r0, r1, r2);
      assert(0);
    }
  }

  {
    int32_t numbers[N * N + 1] = {
      13,  2,  3,
       4, 55,  6,
      -2,  8,  1,
      485
    };
    int32_t r0 = numbers[ANS];
    int32_t r1 = count_det_test(numbers);
    int32_t r2 = count_det(numbers);
    if (r0 != r1 || r0 != r2) {
      printf("Error: r0 = %d, r1 = %d, r2 = %d\n", r0, r1, r2);
      assert(0);
    }
  }
  printf("All tests passed!\n");
}

int main() {
  run_tests();

  int32_t *numbers = malloc(sizeof(int32_t) * 9);
  for (uint8_t i = 0; i < 9; ++i)
    scanf("%d", &numbers[i]);

  const int32_t det = count_det(numbers);
  printf("%d\n", det);
  free(numbers);

  return 0;
}
