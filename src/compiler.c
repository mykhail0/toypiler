#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define POSSIBLE_PROCEDURE_NUM 26

typedef struct {
  int *calls, calls_num, size;
} darr;

/**
 * Check if instructions array needs reallocation and do so if needed.
 */
void reallocate(instruction** instruction, int* n, int address) {
  if (*n == address) {
    *n = more(*n);
    *instruction = realloc(*instruction, (*n) * sizeof **instruction);
    assert(*instruction != NULL);
  }
}

/**
 * Check if dynamic array needs reallocation and do so if needed.
 */
void realloc_darr(darr* arr) {
  if (arr->size == arr->calls_num) {
    arr->size = more(arr->size);
    arr->calls = realloc(arr->calls, (arr->size) * sizeof *(arr->calls));
    assert(arr->calls != NULL);
  }
}

void print(instruction instruction) {
  printf("%d", instruction.a);
  switch (instruction.a) {
    case INPUT_BRANCH:
    case JUMP:
    case CALL:
      printf(" %d", instruction.b);
      break;
    case POP_BRANCH:
      printf(" %d", instruction.b);
      printf(" %d", instruction.c);
      break;
    case PUSH_0:
    case PUSH_1:
      printf(" %d", instruction.c);
      break;
    default:
      break;
  }
  printf("\n");
}

void print_code(instruction instruction[], int n) {
  for (int i = 0; i < n; ++i) print(instruction[i]);
}

static const char SPECIAL_CHARS[] = {'$', '{', '}', '-', '+', '\0'};

void found_unexpected() {
  perror("Found unexpected character.");
  exit(EXIT_FAILURE);
}

void expected(int expected_ch, int actual_ch) {
  if (actual_ch != expected_ch) {
    found_unexpected();
  }
}

int my_getchar() {
  int ch = getchar();
  while (ch != EOF && !isalpha(ch) && strchr(SPECIAL_CHARS, ch) == NULL) {
    if (ch == ';') {
      ch = getchar();
      while (ch != '\n' && ch != EOF) ch = getchar();
    } else if (ch == '\t' || ch == '\n' || ch == ' ') {
      ch = getchar();
    } else {
      perror(
          "Invalid character, valid input is only tab, newline, space, $, {, "
          "}, +, -, ; and alphabetic characters.");
      exit(EXIT_FAILURE);
    }
  }

  if (ch == EOF) {
    if (!feof(stdin) && ferror(stdin)) {
      perror("getchar()");
      exit(EXIT_FAILURE);
    }
  }

  return ch;
}

/**
 * Read an instruction of pushing onto stack number `stack` bits from `ch`.
 * Return the next meaningful character after a sequence of `+` and `-`.
 */
int push(instruction** instruction, int* n, int* address, int ch, int stack) {
  while (ch == '+' || ch == '-') {
    (*instruction)[*address].c = stack - 'a';
    (*instruction)[(*address)++].a = ch == '+' ? PUSH_1 : PUSH_0;
    ch = my_getchar();
    reallocate(instruction, n, *address);
  }
  return ch;
}

/**
 * Read an instruction of outputting a character `ch`.
 * Return the next meaningful character after a sequence of `+` and `-`.
 */
int output(instruction** instruction, int* n, int* address, int ch) {
  while (ch == '+' || ch == '-') {
    (*instruction)[(*address)++].a = ch == '+' ? OUTPUT_1 : OUTPUT_0;
    ch = my_getchar();
    reallocate(instruction, n, *address);
  }
  return ch;
}

void read(instruction**, int*, int*, darr*, int*);

/**
 * Read a choice instruction, including nested instructions.
 */
int ifstatement(instruction** instruction, int* n, int procedure_addrs[],
                darr procedure_calls[], int* address, int ch) {
  if (ch == '$')
    (*instruction)[*address].a = INPUT_BRANCH;
  else {
    (*instruction)[*address].a = POP_BRANCH;
    (*instruction)[*address].c = ch - 'a';
  }

  int ifaddress = (*address)++;
  reallocate(instruction, n, *address);
  read(instruction, n, procedure_addrs, procedure_calls, address);

  (*instruction)[*address].a = JUMP;
  int jumpaddress = (*address)++;
  reallocate(instruction, n, *address);

  // Augment IF with "else's" address.
  (*instruction)[ifaddress].b = *address;
  expected('{', my_getchar());

  read(instruction, n, procedure_addrs, procedure_calls, address);
  // Augment JUMP with address of instruction after "else".
  (*instruction)[jumpaddress].b = *address;

  return my_getchar();
}

/**
 * Read instructions inside braces. Calls to procedures not yet declared are to
 * be augmented later.
 * `**instruction` is a pointer to the instructions array.
 * `*n` is the current size of said array.
 * `procedure_addrs[]` addresses of procedures of every possible name.
 * `procedure_calls[]` each element of the array is a list of calls to the
 * procedure of a given name.
 * `*address` address of the instruction being currently processed.
 */
void read(instruction** instruction, int* n, int procedure_addrs[],
          darr procedure_calls[], int* address) {
  bool keep_going = true;
  int ch = my_getchar();
  while (keep_going && ch != EOF) {
    if (ch == '}') {
      keep_going = false;
    } else {
      int nextch = my_getchar();

      if (('a' <= ch && ch <= 'z') || ch == '$') {
        if (nextch == '{') {
          nextch = ifstatement(instruction, n, procedure_addrs, procedure_calls,
                               address, ch);
        } else if (nextch == '+' || nextch == '-') {
          nextch = ch == '$' ? output(instruction, n, address, nextch)
                             : push(instruction, n, address, nextch, ch);
        } else {
          found_unexpected();
        }
      } else if ('A' <= ch && ch <= 'Z') {
        (*instruction)[*address].a = CALL;
        int name = ch - 'A';
        if (procedure_addrs[name]) {
          (*instruction)[(*address)++].b = procedure_addrs[name];
        } else {
          // Add the call to the list of calls to be augmented later.
          procedure_calls[name].calls[procedure_calls[name].calls_num++] =
              (*address)++;
          realloc_darr(&procedure_calls[name]);
        }
        reallocate(instruction, n, *address);
      } else {
        found_unexpected();
      }

      ch = nextch;
    }
  }
}

int main() {
  int ch = my_getchar(), address = 1, n = 10;

  instruction* instructions;
  instructions = malloc(n * sizeof *instructions);
  assert(instructions != NULL);
  instructions[0].a = JUMP;

  // Initialize array of addresses, where each procedure starts.
  int procedure_addrs[POSSIBLE_PROCEDURE_NUM];
  for (int i = 0; i < POSSIBLE_PROCEDURE_NUM; ++i)
    // At first, the addresses aren't known.
    procedure_addrs[i] = 0;

  // Address of instructions where the procedure was called when it hadn't
  // been initialized yet.
  darr procedure_calls[POSSIBLE_PROCEDURE_NUM];
  for (int i = 0; i < POSSIBLE_PROCEDURE_NUM; ++i) {
    procedure_calls[i].calls_num = 0;
    procedure_calls[i].size = n;
    procedure_calls[i].calls = (int*)malloc(
        procedure_calls[i].size * sizeof(*(procedure_calls[i].calls)));
    assert(procedure_calls[i].calls != NULL);
  }

  // Generate code for every procedure into `instructions`.
  while ('A' <= ch && ch <= 'Z') {
    int name = ch - 'A';
    procedure_addrs[name] = address;
    if (procedure_calls[name].calls_num > 0) {
      for (int i = 0; i < procedure_calls[name].calls_num; ++i)
        instructions[procedure_calls[name].calls[i]].b = procedure_addrs[name];
    }
    expected('{', my_getchar());
    read(&instructions, &n, procedure_addrs, procedure_calls, &address);
    instructions[address++].a = RETURN;
    reallocate(&instructions, &n, address);
    ch = my_getchar();
  }

  expected('{', ch);
  // Augment initial JUMP with "main's" address.
  instructions[0].b = address;
  // Generate code for "main".
  read(&instructions, &n, procedure_addrs, procedure_calls, &address);
  instructions[address].a = HALT;

  // Cleanup.
  for (int i = 0; i < POSSIBLE_PROCEDURE_NUM; ++i)
    free(procedure_calls[i].calls);
  print_code(instructions, address + 1);
  free(instructions);

  return 0;
}
