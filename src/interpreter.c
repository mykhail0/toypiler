#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define START 0

#define BITS 8

#define BASE 10

#define EMPTY (-1)

#define MY_EOF 255

typedef struct {
  int height;
  int size;
  int* elements;
} stack;

void initiate(stack* s) {
  s->height = 0;
  s->size = 0;
  s->elements = NULL;
}

void clear(stack* s) { free(s->elements); }

void push(stack* s, int x) {
  if (s->height == s->size) {
    s->size = more(s->size);
    int* new_elements = realloc(s->elements, s->size * sizeof *new_elements);
    assert(new_elements != NULL);
    s->elements = new_elements;
  }
  s->elements[s->height++] = x;
}

int pop(stack* s) {
  assert(s->height > 0);
  return s->elements[--s->height];
}

void append(instruction** p, int* n, int* z, opcode a, int b, int c) {
  if (*n == *z) {
    *z = more(*z);
    instruction* new_instruction = realloc(*p, *z * sizeof *new_instruction);
    assert(new_instruction != NULL);
    *p = new_instruction;
  }
  (*p)[*n].a = a;
  (*p)[*n].b = b;
  (*p)[*n].c = c;
  ++*n;
}

bool whitespace(int c) { return (c == ' ') || (c == '\n') || (c == '\t'); }

int next(void) {
  int c;
  do {
    c = getchar();
  } while (whitespace(c));
  if (c == EOF) {
    return EMPTY;
  }
  assert(isdigit(c));
  int result = 0;
  while (isdigit(c)) {
    assert(result <= (INT_MAX - c + '0') / BASE);
    result = result * BASE + c - '0';
    c = getchar();
  }
  ungetc(c, stdin);
  return result;
}

void read(instruction** p, int* n, int* s) {
  *p = NULL;
  int z = 0;
  *n = 0;
  *s = 0;
  int m = EMPTY;
  int a;
  bool end = false;
  while (!end) {
    a = next();
    assert(a != EMPTY);
    int b = EMPTY;
    int c = EMPTY;
    switch (a) {
      case OUTPUT_0:
      case OUTPUT_1:
      case RETURN:
        break;
      case HALT:
        end = true;
        break;
      case PUSH_0:
      case PUSH_1:
        b = next();
        assert(b != EMPTY);
        if (b >= *s) {
          *s = b + 1;
        }
        break;
      case INPUT_BRANCH:
      case JUMP:
      case CALL:
        b = next();
        assert(b != EMPTY);
        if (b > m) {
          m = b;
        }
        break;
      case POP_BRANCH:
        b = next();
        assert(b != EMPTY);
        if (b > m) {
          m = b;
        }
        c = next();
        assert(c != EMPTY);
        if (c >= *s) {
          *s = c + 1;
        }
        break;
      default:
        assert(false);
        break;
    }
    append(p, n, &z, a, b, c);
  }
  int c = getchar();
  assert(c == '\n');
  assert(m <= (*n) - 1);
}

void allocate_memory(stack** a, int s) {
  *a = malloc(s * sizeof **a);
  assert(*a != NULL);
  for (int i = 0; i < s; ++i) {
    initiate(&(*a)[i]);
  }
}

void free_memory(stack* a, int s) {
  for (int i = 0; i < s; ++i) {
    clear(&a[i]);
  }
  free(a);
}

int read_bit(int* a, int* m) {
  if (*m == 0) {
    int c = getchar();
    if (c == EOF) {
      *a = MY_EOF;
    } else {
      *a = c;
    }
    *m = 1 << (BITS - 1);
  }
  int result = ((*a & *m) != 0);
  *m >>= 1;
  return result;
}

void write_bit(int* a, int* m, int b) {
  if (*m == 0) {
    *a = 0;
    *m = 1 << (BITS - 1);
  }
  if (b != 0) {
    *a |= *m;
  }
  *m >>= 1;
  if (*m == 0) {
    putchar(*a);
  }
}

void execute(instruction p[], int s) {
  stack* memory;
  allocate_memory(&memory, s);
  stack returns;
  initiate(&returns);
  int ip = START;
  bool end = false;
  int bit;
  int current_input_character = 0;
  int input_mask = 0;
  int current_output_character = 0;
  int output_mask = 1 << (BITS - 1);
  while (!end) {
    opcode a = p[ip].a;
    switch (a) {
      case PUSH_0:
      case PUSH_1:
        if (a == PUSH_0) {
          bit = 0;
        } else {
          bit = 1;
        }
        push(&memory[p[ip].b], bit);
        break;
      case OUTPUT_0:
      case OUTPUT_1:
        if (a == OUTPUT_0) {
          bit = 0;
        } else {
          bit = 1;
        }
        write_bit(&current_output_character, &output_mask, bit);
        break;
      case POP_BRANCH:
      case INPUT_BRANCH:
        if (a == POP_BRANCH) {
          bit = pop(&memory[p[ip].c]);
        } else {
          bit = read_bit(&current_input_character, &input_mask);
        }
        if (bit == 0) {
          ip = p[ip].b;
          continue;
        } else {
          break;
        }
      case CALL:
        push(&returns, ip + 1);
        ip = p[ip].b;
        continue;
      case JUMP:
        ip = p[ip].b;
        continue;
      case RETURN:
        ip = pop(&returns);
        continue;
      case HALT:
        if (output_mask != 0) {
          putchar(current_output_character);
        }
        end = true;
        continue;
      default:
        assert(false);
        break;
    }
    ++ip;
  }
  free_memory(memory, s);
  clear(&returns);
}

int main(void) {
  instruction* p;
  int n;
  int s;
  read(&p, &n, &s);
  execute(p, s);
  free(p);
  return 0;
}
