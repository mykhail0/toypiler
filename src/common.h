#define MULTIPLIER 3
#define DIVISOR 2

typedef enum {
  PUSH_0 = 0,
  PUSH_1 = 1,
  OUTPUT_0 = 2,
  OUTPUT_1 = 3,
  POP_BRANCH = 4,
  INPUT_BRANCH = 5,
  JUMP = 6,
  CALL = 7,
  RETURN = 8,
  HALT = 9
} opcode;

typedef struct {
  opcode a;
  int b;
  int c;
} instruction;

int more(int);
