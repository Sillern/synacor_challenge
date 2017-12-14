#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILENAME_LENGTH 256
#define MEMORY_SPACE 0x7FFF
#define NUM_REGISTERS 8U
#define MAX_NUMBER 32768

static uint16_t registers[NUM_REGISTERS] = {0};

struct operand_t
{
  uint16_t direct;
  uint16_t* indirect;
};

struct instruction_t
{
  uint16_t opcode;
  uint16_t argc;

  struct operand_t a;
  struct operand_t b;
  struct operand_t c;
};

struct stack_t
{
  struct stack_t* previous;
  uint16_t value;
};

enum OPCODES
{
  HALT = 0,
  SET = 1,
  PUSH = 2,
  POP = 3,
  EQ = 4,
  GT = 5,
  JMP = 6,
  JT = 7,
  JF = 8,
  ADD = 9,
  MULT = 10,
  MOD = 11,
  AND = 12,
  OR = 13,
  NOT = 14,
  RMEM = 15,
  WMEM = 16,
  CALL = 17,
  RET = 18,
  OUT = 19,
  IN = 20,
  NOOP = 21
};

uint8_t is_value(uint16_t value)
{
  return value < 32768U;
}

uint8_t is_register(uint16_t value)
{
  return value > 32767U && value < 32776;
}

uint16_t get_register_index(uint16_t value)
{
  return (32768U - value) % NUM_REGISTERS;
}

uint16_t num_arguments(uint16_t opcode)
{
  const uint16_t arguments[] = {
    0, 2, 1, 1, 3, 3,
    1, 2, 2, 3, 3, 3,
    3, 3, 2, 2, 2, 1,
    0, 1, 1, 0};

  return arguments[opcode];
}

uint16_t get_operand_value(struct operand_t* operand)
{
  if (is_register(operand->direct))
  {
    return *operand->indirect;
  }
  return operand->direct;
}

void set_operand(struct operand_t* operand, uint16_t value)
{
  operand->direct = value;
  if (is_register(value))
  {
    operand->indirect = &registers[get_register_index(operand->direct)];
  }
}

void get_instruction(uint16_t* program, uint16_t program_counter, struct instruction_t* instruction)
{
  if (instruction == 0)
  {
    return;
  }

  instruction->opcode = program[program_counter];
  instruction->argc = num_arguments(instruction->opcode);
  /* Fallthrough */
  switch (instruction->argc)
  {
    case 3:
      {
        set_operand(&instruction->c, program[program_counter + 3]);
      }
    case 2:
      {
        set_operand(&instruction->b, program[program_counter + 2]);
      }
    case 1:
      {
        set_operand(&instruction->a, program[program_counter + 1]);
      }
      break;
    case 0:
      break;
  }
}

uint8_t operand_is_register(struct operand_t* operand)
{
  if (is_register(operand->direct) == 0)
  {
    printf("Invalid register: %d\n", operand->direct);
    return 0;
  }
  return 1;
}

void print_operand(struct operand_t* operand)
{
  if (is_register(operand->direct))
  {
    fprintf(stderr, "[%d]", get_register_index(operand->direct));
  }
  else
  {
    fprintf(stderr, "0x%04X", operand->direct);
  }
}

void print_opcode(struct instruction_t* instruction, uint16_t address, uint16_t indentation)
{
  const char* opcodes[] = {
    "HALT", "SET", "PUSH", "POP", "EQ", "GT",
    "JMP", "JT", "JF", "ADD", "MULT", "MOD",
    "AND", "OR", "NOT", "RMEM", "WMEM",
    "CALL", "RET", "OUT", "IN", "NOOP"};

  if (instruction == 0)
  {
    printf("Invalid memory for instruction\n");
    return;
  }

  for (uint16_t i = 0; i < indentation; ++i)
  {
    fprintf(stderr, "  ");
  }
  fprintf(stderr, "0x%04X: %s ", address, opcodes[instruction->opcode]);
  switch (instruction->argc)
  {
    case 0:
      break;
    case 1:
      print_operand(&instruction->a);
      break;
    case 2:
      print_operand(&instruction->a);
      fprintf(stderr, " ");
      print_operand(&instruction->b);
      break;
    case 3:
      print_operand(&instruction->a);
      fprintf(stderr, " ");
      print_operand(&instruction->b);
      fprintf(stderr, " ");
      print_operand(&instruction->c);
      break;
  }
  fprintf(stderr, "\n");
}

uint8_t is_stack_empty(struct stack_t* stack)
{
  if (stack == 0)
  {
    return 1;
  }
  return 0;
}

uint16_t pop_stack(struct stack_t** stack_pp)
{
  if (stack_pp == 0 && stack_pp == 0)
  {
    printf("Empty stack\n");
    return 0xFFFF;
  }

  uint16_t value = (*stack_pp)->value;
  struct stack_t* previous_stack = (*stack_pp)->previous;
  free(*stack_pp);
  *stack_pp = previous_stack;
  return value;
}

void push_stack(struct stack_t** stack_pp, uint16_t value)
{
  struct stack_t* new_stack = (struct stack_t*)malloc(sizeof(struct stack_t));
  new_stack->value = value;
  new_stack->previous = *stack_pp;
  *stack_pp = new_stack;
}

uint16_t load_program(uint16_t* memory, const char* filename)
{
  FILE* filehandle = fopen(filename, "rb");

  fseek(filehandle, 0L, SEEK_END);
  const long filesize = ftell(filehandle);
  rewind(filehandle);

  if (filesize > (MEMORY_SPACE * 2))
  {
    printf("Supplied program size too large, %ld bytes\n", filesize);
    return 0;
  }

  const uint16_t expected_size = (uint16_t)filesize / 2U;
  const uint16_t read_words = fread(memory, 2, expected_size, filehandle);
  fclose(filehandle);

  if ((filesize / 2) != read_words)
  {
    printf("Odd filesize\n");
		return 0;
  }

	return read_words;
}

void disassemble(uint16_t* memory, uint16_t offset, uint16_t length)
{
  uint16_t program_counter = offset;
  uint8_t found_string = 0;
  char string[1024] = {0};
  uint16_t stringindex = 0;

  while (program_counter < MEMORY_SPACE && program_counter < (offset + length))
  {
    struct instruction_t instruction;
    get_instruction(memory, program_counter, &instruction);

    if (instruction.opcode == OUT)
    {
      found_string = 1;
      string[stringindex] = instruction.a.direct;
      ++stringindex;
    }
    else
    {
      if (found_string == 1)
      {
        printf("STRING: %s\n", string);
        memset(string, 0U, 1024);
        stringindex = 0;
      }
      found_string = 0;
    }

    print_opcode(&instruction, program_counter, 0);
    program_counter += 1 + instruction.argc;
  }
}

void virtual_machine(const char* filename, uint8_t print_instructions)
{
  uint16_t memory[MEMORY_SPACE];

	const uint16_t programsize = load_program(memory, filename);
	if (programsize == 0)
	{
		printf("Invalid program\n");
		return;
	}

  /*
  disassemble(memory, 0x1545, 0x40);
  */

  uint16_t indentation = 0;
	uint8_t halt = 0;
  struct stack_t* stack = 0;
  uint16_t program_counter = 0;
  while (!halt)
  {
    struct instruction_t instruction;
    get_instruction(memory, program_counter, &instruction);

    if (print_instructions)
    {
      print_opcode(&instruction, program_counter, indentation);
    }

    program_counter += 1 + instruction.argc;

    switch (instruction.opcode)
    {
      /* stop execution and terminate the program */
      case HALT:
      {
				halt = 1;
				continue;
      }
      break;

      /* set: 1 a b */
      /* set register <a> to the value of <b> */
      case SET:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = get_operand_value(&instruction.b);
        continue;
      }
      break;

      /* push: 2 a*/
      /*   push <a> onto the stack*/
      case PUSH:
      {
        push_stack(&stack, get_operand_value(&instruction.a));
        continue;
      }
      break;

      /* pop: 3 a*/
      /*   remove the top element from the stack and write it into <a>; empty stack = error*/
      case POP:
      {
        *instruction.a.indirect = pop_stack(&stack);
        continue;
      }
      break;

      /* eq: 4 a b c*/
      /*   set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise*/
      case EQ:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = (get_operand_value(&instruction.b) == get_operand_value(&instruction.c));
        continue;
      }
      break;

      /* gt: 5 a b c*/
      /*   set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise*/
      case GT:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = (get_operand_value(&instruction.b) > get_operand_value(&instruction.c));
        continue;
      }
      break;

      /* jmp: 6 a*/
      /*   jump to <a>*/
      case JMP:
      {
        program_counter = get_operand_value(&instruction.a);
        continue;
      }
      break;

      /* jt: 7 a b*/
      /*   if <a> is nonzero, jump to <b>*/
      case JT:
      {
        if (get_operand_value(&instruction.a) != 0)
        {
          program_counter = get_operand_value(&instruction.b);
        }
        continue;
      }
      break;

      /* jf: 8 a b*/
      /*   if <a> is zero, jump to <b>*/
      case JF:
      {
        if (get_operand_value(&instruction.a) == 0)
        {
          program_counter = get_operand_value(&instruction.b);
        }
        continue;
      }
      break;

      /* add: 9 a b c*/
      /*   assign into <a> the sum of <b> and <c> (modulo 32768)*/
      case ADD:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = ((get_operand_value(&instruction.b) +
                                    get_operand_value(&instruction.c)) % MAX_NUMBER);
        continue;
      }
      break;

      /* mult: 10 a b c*/
      /*   store into <a> the product of <b> and <c> (modulo 32768)*/
      case MULT:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = ((get_operand_value(&instruction.b) *
                                    get_operand_value(&instruction.c)) % MAX_NUMBER);
        continue;
      }
      break;

      /* mod: 11 a b c*/
      /*   store into <a> the remainder of <b> divided by <c>*/
      case MOD:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = (get_operand_value(&instruction.b) % get_operand_value(&instruction.c));
        continue;
      }
      break;

      /* and: 12 a b c*/
      /*   stores into <a> the bitwise and of <b> and <c>*/
      case AND:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = (get_operand_value(&instruction.b) & get_operand_value(&instruction.c));
        continue;
      }
      break;

      /* or: 13 a b c*/
      /*   stores into <a> the bitwise or of <b> and <c>*/
      case OR:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = (get_operand_value(&instruction.b) | get_operand_value(&instruction.c));
        continue;
      }
      break;

      /* not: 14 a b*/
      /*   stores 15-bit bitwise inverse of <b> in <a>*/
      case NOT:
      {
        if (!operand_is_register(&instruction.a))
        {
          return;
        }

        *instruction.a.indirect = get_operand_value(&instruction.b) ^ 0x7FFF;
        continue;
      }
      break;

      /* rmem: 15 a b*/
      /*   read memory at address <b> and write it to <a>*/
      case RMEM:
      {
        *instruction.a.indirect = memory[get_operand_value(&instruction.b)];
        continue;
      }
      break;

      /* wmem: 16 a b*/
      /*   write the value from <b> into memory at address <a>*/
      case WMEM:
      {
        memory[get_operand_value(&instruction.a)] = get_operand_value(&instruction.b);
        continue;
      }
      break;

      /* call: 17 a*/
      /*   write the address of the next instruction to the stack and jump to <a>*/
      case CALL:
      {
        ++indentation;
        push_stack(&stack, program_counter);
        program_counter = get_operand_value(&instruction.a);
        continue;
      }
      break;

      /* ret: 18*/
      /*   remove the top element from the stack and jump to it; empty stack = halt*/
      case RET:
      {
        --indentation;
        if (is_stack_empty(stack))
				{
          halt = 1;
        }
				else
        {
					program_counter = pop_stack(&stack);
				}
        continue;
      }
      break;

      /* out: 19 a*/
      /*   write the character represented by ascii code <a> to the terminal*/
      case OUT:
      {
        printf("%c", get_operand_value(&instruction.a));
        continue;
      }
      break;

      /* in: 20 a*/
      /*   read a character from the terminal and write its ascii code to <a>; it can be assumed that once input starts, it will continue until a newline is encountered; this means that you can safely read whole lines from the keyboard and trust that they will be fully read*/
      case IN:
      {
        *instruction.a.indirect = getc(stdin);
        continue;
      }
      break;

      /* noop: 21*/
      /*   no operation*/
      case NOOP:
      {
        continue;
      }
      break;
    }

    printf("UNKNOWN OPCODE\n");
    print_opcode(&instruction, program_counter, indentation);
    break;
  }
}

int main(int32_t argc, char** argv)
{
  uint8_t print_instructions = 0U;

  if (argc < 2)
  {
    printf("No program supplied\n");
    return 1;
  }

  if (argc > 2)
  {
    print_instructions = strtoul(argv[2], 0, 10);
  }

  char filename[MAX_FILENAME_LENGTH] = {0};

  strncpy(filename, argv[1], MAX_FILENAME_LENGTH);
  printf("Starting virtual machine with %s\n", filename);

  virtual_machine(filename, print_instructions);

  return 0;
}
