#include<stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* vector with small buffer optimization */

#define STRUCT_BODY(type)                                                      \
  struct {                                                                     \
    size_t size : 54, on_heap : 1, capacity : 6, flag1 : 1, flag2 : 1,         \
        flag3 : 1;                                                             \
    type *ptr;                                                                 \
  }

#define NEXT_POWER_OF_2(s)                                                     \
   (s & (s - 1)) == 0 ? s : (size_t)1 << (64 - __builtin_clzl(s)) /* next power of 2 */

#define v(t, s, name, ...)                                                     \
  union {                                                                      \
    STRUCT_BODY(t);                                                            \
    struct {                                                                   \
      size_t filler;                                                           \
      t buf[NEXT_POWER_OF_2(s)];                                               \
    };                                                                         \
  } name __attribute__((cleanup(vec_free))) = {.buf = {__VA_ARGS__}};          \
  name.size = sizeof((__typeof__(name.buf[0])[]){0, __VA_ARGS__}) /            \
                  sizeof(name.buf[0]) -                                        \
              1;                                                               \
  name.capacity = sizeof(name.buf) / sizeof(name.buf[0])

#define vec_size(v) v.size
#define vec_capacity(v)                                                        \
  (v.on_heap ? (size_t)1 << v.capacity : sizeof(v.buf) / sizeof(v.buf[0]))

#define vec_data(v) (v.on_heap ? v.ptr : v.buf) /* always contiguous buffer */

#define vec_elemsize(v) sizeof(v.buf[0])
#define vec_pos(v, n) vec_data(v)[n] /* lvalue */

#define vec_reserve(v, n) __vec_reserve(&v, n, vec_elemsize(v), vec_capacity(v))
#define vec_push_back(v, e)                                                    \
  __vec_push_back(&v, &(__typeof__(v.buf[0])[]){e}, vec_elemsize(v),           \
                  vec_capacity(v))

#define vec_pop_back(v) (void)(v.size -= 1)

/* This function attribute specifies function parameters that are not supposed
 * to be null pointers. This enables the compiler to generate a warning on
 * encountering such a parameter.
 */
#define NON_NULL __attribute__((nonnull))

    static NON_NULL void vec_free(void *p) {
  STRUCT_BODY(void) *s = p;
  if (s->on_heap)
    free(s->ptr);
}

static inline int ilog2(size_t n) {
  return 64 - __builtin_clzl(n) - ((n & (n - 1)) == 0);
}

static NON_NULL void __vec_reserve(void *vec, size_t n, size_t elemsize,
                                   size_t capacity) {
  union {
    STRUCT_BODY(void);
    struct {
      size_t filler;
      char buf[];
    };
  } *v = vec;

  if (n > capacity) {
    if (v->on_heap) {
      v->ptr =
          realloc(v->ptr, elemsize * (size_t)1 << (v->capacity = ilog2(n)));
    } else {
      void *tmp = malloc(elemsize * (size_t)1 << (v->capacity = ilog2(n)));
      memcpy(tmp, v->buf, elemsize * v->size);
      v->ptr = tmp;
      v->on_heap = 1;
    }
  }
}

static NON_NULL void __vec_push_back(void *restrict vec, void *restrict e,
                                     size_t elemsize, size_t capacity) {
  union {
    STRUCT_BODY(char);
    struct {
      size_t filler;
      char buf[];
    };
  } *v = vec;

  if (v->on_heap) {
    if (v->size == capacity)
      v->ptr = realloc(v->ptr, elemsize * (size_t)1 << ++v->capacity);
    memcpy(&v->ptr[v->size++ * elemsize], e, elemsize);
  } else {
    if (v->size == capacity) {
      void *tmp = malloc(elemsize * (size_t)1 << (v->capacity = capacity + 1));
      memcpy(tmp, v->buf, elemsize * v->size);
      v->ptr = tmp;
      v->on_heap = 1;
      memcpy(&v->ptr[v->size++ * elemsize], e, elemsize);
    } else
      memcpy(&v->buf[v->size++ * elemsize], e, elemsize);
  }
}

int main() {
  v(float, 3, vec1);
  v(int, 2, vec2, 13, 42);

  printf("pos(vec2,0)=%d, pos(vec2,1)=%d\n", vec_pos(vec2, 0),
         vec_pos(vec2, 1));
  vec_push_back(vec2, 88);
  vec_reserve(vec2, 100);
  printf("capacity(vec1)=%zu, capacity(vec2)=%zu\n", vec_capacity(vec1),
         vec_capacity(vec2));
  printf("pos(vec2,2)=%d\n", vec_pos(vec2, 2));

#define display(v)                                                             \
  do {                                                                         \
    for (size_t i = 0; i < vec_size(v); i++)                                   \
      printf("%.2f ", vec_pos(v, i));                                          \
    puts(v.on_heap ? "heap" : "stack");                                        \
  } while (0)

  display(vec1);
  vec_push_back(vec1, 0.0);
  display(vec1);
  vec_push_back(vec1, 1.1);
  display(vec1);
  vec_push_back(vec1, 2.2);
  display(vec1);
  vec_push_back(vec1, 3.3);
  display(vec1);
  vec_push_back(vec1, 4.4);
  display(vec1);
  vec_push_back(vec1, 5.5);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);
  vec_pop_back(vec1);
  display(vec1);

  return 0;
}
