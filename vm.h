//
// Created by zbh on 2019/12/9.
//

#ifndef TPL3_VM_H
#define TPL3_VM_H

#include <stdint.h>

extern const int INT_LEN;
const int PTR_LEN;
const int FLOAT_LEN;
const int CHAR_LEN;
const int BOOLEAN_LEN;
const int VOID_LEN;

void vm_load(const unsigned char *codes, int read);

void print_memory();

#endif //TPL3_VM_H
