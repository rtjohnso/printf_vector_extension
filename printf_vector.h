/* Copyright 2019 Rob Johnson <rtjohnsocs42@gmail.com> */
/* BSD License. */

#ifndef PRINTF_VECTOR_H
#define PRINTF_VECTOR_H

#include <printf.h>

int printf_vector(FILE *stream, 
                  const struct printf_info *info,
                  const void * const *args);

int printf_vector_arginfo_size (const struct printf_info *info, 
                                size_t n,
                                int *argtypes,
                                int *size);

#endif
