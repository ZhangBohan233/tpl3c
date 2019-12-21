//
// Created by zbh on 2019/12/9.
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vm.h"
#include "lib.h"

const int INT_LEN = 8;
const int PTR_LEN = 8;
const int FLOAT_LEN = 8;
const int CHAR_LEN = 1;
const int BOOLEAN_LEN = 1;
const int VOID_LEN = 0;

const int64_t STACK_START = 1;
int64_t LITERAL_START = 1024;
int64_t FUNCTIONS_START = 1024;
int64_t CODE_START = 1024;
int64_t HEAP_START = 1024;

const int64_t MEMORY_SIZE = 4096;
unsigned char MEMORY[4096];

int64_t SP = 1;
int64_t PC = 1024;

int CALL_STACK[1000];  // recursion limit
int CSP = -1;

int PC_STACK[1000];
int PSP = -1;

void print_memory() {
    int i = 0;
    printf("Stack ");
    for (; i < LITERAL_START; i++) {
        printf("%d ", MEMORY[i]);
        if (i % 8 == 0) printf("| ");
    }

    printf("\nLiteral %lld: ", LITERAL_START);
    for (; i < FUNCTIONS_START; i++) {
        printf("%d ", MEMORY[i]);
    }

    printf("\nFunctions %lld: ", FUNCTIONS_START);
    for (; i < CODE_START; i++) {
        printf("%d ", MEMORY[i]);
    }

    printf("\nCode %lld: ", CODE_START);
    for (; i < HEAP_START; i++) {
        printf("%d ", MEMORY[i]);
    }

//    printf("\nHeap %d: ", HEAP_START);
//    for (; i < MEMORY_SIZE; i++) {
//        printf("%d ", MEMORY[i]);
//    }
    printf("\n");
}

void print_call_stack() {
    printf("Call stack: ");
    for (int i = 0; i <= CSP; i++) {
        printf("%d, ", CALL_STACK[CSP]);
    }
    printf("\n");
}

void vm_load(const unsigned char *codes, int read) {
    int64_t literal_size = bytes_to_int(codes);
    int64_t functions_size = bytes_to_int(codes + INT_LEN);

    FUNCTIONS_START += literal_size;
    CODE_START = FUNCTIONS_START + functions_size;
    PC = CODE_START;

    int copy_len = read - INT_LEN * 2;
    HEAP_START = LITERAL_START + copy_len;

    memcpy(MEMORY + LITERAL_START, codes + INT_LEN * 2, copy_len);

//    print_memory();
}

void mem_copy(int64_t from, int64_t to, int64_t len) {
    memcpy(MEMORY + to, MEMORY + from, len);
}

void exit_func() {
    SP = CALL_STACK[CSP--];
    PC = PC_STACK[PSP--];
}

int64_t true_ptr(int64_t ptr) {
    if (ptr < LITERAL_START && CSP >= 0) {
        return ptr + CALL_STACK[CSP];
    }
    if (ptr >= MEMORY_SIZE) {
        fprintf(stderr, "\nCannot access address %lld\n", ptr);
        return 0;
    }
    return ptr;
}

void read_2_ints(int64_t *reg1, int64_t *reg2) {
    *reg1 = bytes_to_int(MEMORY + PC);
    *reg2 = bytes_to_int(MEMORY + PC + INT_LEN);
    PC += INT_LEN * 2;
}

void read_3_ints(int64_t *reg1, int64_t *reg2, int64_t *reg3) {
    *reg1 = bytes_to_int(MEMORY + PC);
    *reg2 = bytes_to_int(MEMORY + PC + INT_LEN);
    *reg3 = bytes_to_int(MEMORY + PC + INT_LEN * 2);
    PC += INT_LEN * 3;
}

void read_4_ints(int64_t *reg1, int64_t *reg2, int64_t *reg3, int64_t *reg4) {
    *reg1 = bytes_to_int(MEMORY + PC);
    *reg2 = bytes_to_int(MEMORY + PC + INT_LEN);
    *reg3 = bytes_to_int(MEMORY + PC + INT_LEN * 2);
    *reg4 = bytes_to_int(MEMORY + PC + INT_LEN * 3);
    PC += INT_LEN * 4;
}

void read_3_true_ptr(int64_t *reg1, int64_t *reg2, int64_t *reg3) {
    read_3_ints(reg1, reg2, reg3);
    *reg1 = true_ptr(*reg1);
    *reg2 = true_ptr(*reg2);
    *reg3 = true_ptr(*reg3);
}

void int_cmp(int64_t *reg1, int64_t *reg2, int64_t *reg3, int64_t *reg4) {
    read_3_true_ptr(reg1, reg2, reg3);
}

void call_native(int64_t func, int64_t ret_ptr, int64_t ret_len, int64_t arg_count, int64_t *arg_array) {
    switch (func) {
        case 1:  // clock
            int64_t t = clock();
            if (arg_count != 0 || ret_len != INT_LEN) {
                printf("Unmatched arg length or return length");
                return;
            }
            int_to_bytes(MEMORY + ret_ptr, t);
            break;
        case 2:
            break;
        default:
            printf("Unknown native function %lld", func);
            return;
    }
}

void vm_run() {
    int64_t reg1;
    int64_t reg2;
    int64_t reg3;
    int64_t reg4;
    int64_t reg5;
    int64_t reg6;
    int64_t reg7;
    int64_t reg8;

    int reg9;
    int reg10;

    unsigned char reg11;
    unsigned char reg12;

    unsigned char instruction;

    while (PC < HEAP_START) {
        instruction = MEMORY[PC++];
        switch (instruction) {
            case 2:  // Stop
                exit_func();
                break;
            case 3:  // ASSIGN
                read_3_ints(&reg1, &reg2, &reg3);  // tar, src, len
                reg4 = true_ptr(reg1);  // true tar
                reg5 = true_ptr(reg2);  // true src
                mem_copy(reg5, reg4, reg3);
                break;
            case 4:  // CALL
                read_3_ints(&reg1, &reg2, &reg3);  // func_ptr, r_len, arg_count
                reg4 = PC;  // pc backup
                PC += reg3 * (INT_LEN + PTR_LEN);
                reg5 = SP;  // sp backup

                for (reg9 = 0; reg9 < reg3; reg9++) {
                    reg6 = bytes_to_int(MEMORY + reg4);  // arg_ptr
                    reg4 += PTR_LEN;
                    reg7 = bytes_to_int(MEMORY + reg4);  // arg_len
                    reg4 += INT_LEN;
                    reg8 = true_ptr(reg6);  // true arg ptr
                    mem_copy(reg8, SP, reg7);
                    SP += reg7;
                }

                PC_STACK[++PSP] = PC;
                CALL_STACK[++CSP] = reg5;

                PC = reg1;
                break;
            case 5:  // RETURN
                read_2_ints(&reg1, &reg2);  // value ptr, rtype len
                reg3 = true_ptr(reg1);  // true value ptr

                reg4 = CALL_STACK[CSP] - reg2;  // where to put the return value
                mem_copy(reg3, reg4, reg2);

                exit_func();
                break;
            case 6:  // GOTO
                reg1 = bytes_to_int(MEMORY + PC);
                PC += INT_LEN;
                PC += reg1;
                break;
            case 7:  // PUSH STACK
                reg1 = bytes_to_int(MEMORY + PC);
                PC += INT_LEN;
                SP += reg1;
                break;
            case 8:  // ASSIGN_I
                read_2_ints(&reg1, &reg2);
                reg1 = true_ptr(reg1);
                int_to_bytes(MEMORY + reg1, reg2);
//                printf("%lld %lld\n", reg1, reg2);
                break;
            case 10:  // ADD INT
                read_3_true_ptr(&reg1, &reg2, &reg3);  // res ptr, left ptr, right ptr
                reg4 = bytes_to_int(MEMORY + reg2);  // left value
                reg5 = bytes_to_int(MEMORY + reg3);  // right value
                reg6 = reg4 + reg5;  // result
                int_to_bytes(MEMORY + reg1, reg6);
                break;
            case 12:  // SUB INT
                read_3_true_ptr(&reg1, &reg2, &reg3);  // res ptr, left ptr, right ptr
                reg4 = bytes_to_int(MEMORY + reg2);  // left value
                reg5 = bytes_to_int(MEMORY + reg3);  // right value
                reg6 = reg4 - reg5;  // result
                int_to_bytes(MEMORY + reg1, reg6);
                break;
            case 16:  // EQ
                read_3_true_ptr(&reg1, &reg2, &reg3);  // res ptr, left ptr, right ptr
                reg4 = bytes_to_int(MEMORY + reg2);  // left value
                reg5 = bytes_to_int(MEMORY + reg3);  // right value
                reg6 = reg4 - reg5;  // cmp_result

                reg11 = reg6 == 0 ? 1 : 0;
                MEMORY[reg1] = reg11;
                break;
            case 17:  // GT
                read_3_true_ptr(&reg1, &reg2, &reg3);  // res ptr, left ptr, right ptr
                reg4 = bytes_to_int(MEMORY + reg2);  // left value
                reg5 = bytes_to_int(MEMORY + reg3);  // right value
                reg6 = reg4 - reg5;  // cmp_result

                reg11 = reg6 > 0 ? 1 : 0;
                MEMORY[reg1] = reg11;
                break;
            case 18:  // LT
                read_3_true_ptr(&reg1, &reg2, &reg3);  // res ptr, left ptr, right ptr
                reg4 = bytes_to_int(MEMORY + reg2);  // left value
                reg5 = bytes_to_int(MEMORY + reg3);  // right value
                reg6 = reg4 - reg5;  // cmp_result

                reg11 = reg6 < 0 ? 1 : 0;
                MEMORY[reg1] = reg11;
                break;
            case 30:  // IF ZERO GOTO
                read_2_ints(&reg1, &reg2);  // skip len, cond ptr
                reg3 = true_ptr(reg2);  // true cond ptr

                reg11 = MEMORY[reg3];
                if (reg11 == 0) {
                    PC += reg1;
                }
                break;
            case 31:  // NATIVE CALL
                read_4_ints(&reg1, &reg2, &reg3, &reg4);  // func ptr, rtype len, r ptr, arg count
                reg5 = bytes_to_int(MEMORY + reg1);  // function code
                reg3 = true_ptr(reg3);  // true return ptr
                reg6 = PC;  // PC backup
                PC += reg4 * (INT_LEN + PTR_LEN);

                int64_t *args = malloc(reg4 * 16);
                for (reg9 = 0; reg9 < reg4; reg9++) {
                    reg7 = bytes_to_int(MEMORY + reg6);  // arg_ptr
                    reg6 += PTR_LEN;
                    reg8 = bytes_to_int(MEMORY + reg6);  // arg_len
                    reg6 += INT_LEN;
                    reg7 = true_ptr(reg7);  // true arg ptr
                    args[reg9 * 2] = reg7;
                    args[reg9 * 2 + 1] = reg8;
                }
                call_native(reg5, reg3, reg2, reg4, args);

                free(args);
                break;
//            case 32:  // STORE ADDR
//                read_2_ints(&reg1, &reg2);  // tar pos, relative addr
//                reg1 = true_ptr(reg1);
//                reg2 = true_ptr(reg2);
//                int_to_bytes(MEMORY + reg1, reg2);
//                break;
            case 33:  // UNPACK ADDR
                read_3_ints(&reg1, &reg2, &reg3);  // result ptr, pointer address, length
                reg1 = true_ptr(reg1);
                reg2 = true_ptr(reg2);   // address storing the address
                reg4 = bytes_to_int(MEMORY + reg2);  // value address
                reg4 = true_ptr(reg4);
                printf("%lld %lld\n", reg2, reg4);
                mem_copy(reg4, reg1, reg3);
                break;
            case 34:  // PTR ASSIGN
                read_3_ints(&reg1, &reg2, &reg3);  // address of ptr, src, len
                reg1 = true_ptr(reg1);
                reg2 = true_ptr(reg2);
                reg4 = bytes_to_int(MEMORY + reg1);  // address of value
                reg4 = true_ptr(reg4);
                printf("%lld %lld %lld\n", reg1, reg2, reg4);
                mem_copy(reg2, reg4, reg3);
                break;
            default:
                printf("Unknown instruction %d\n", instruction);
                return;
        }
    }
}

int main(int argc, char **argv) {
    int read;

    unsigned char *codes = read_file(argv[1], &read);
//    printf("%d\n", sizeof(unsigned char));

    vm_load(codes, read);

    vm_run();

    print_memory();
    printf("Process finished with exit code %lld\n", bytes_to_int(MEMORY + 1));
}