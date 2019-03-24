/* Copyright 2019 Rob Johnson <rtjohnsocs42@gmail.com> */
/* BSD License. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <printf.h>
#include <printf_vector.h>

/******************************
 * printf extension for arrays
 ******************************/

static int pa2size(const struct printf_info *info, int pa)
{
  if (pa & PA_FLAG_PTR)
    return sizeof(void *);

  switch (pa & (~PA_FLAG_MASK)) {
  case PA_INT:
    switch (pa & PA_FLAG_MASK) {
    case 0: return sizeof(int);
    case PA_FLAG_LONG_LONG: return sizeof(long long);
    case PA_FLAG_LONG: return sizeof(long);
    case PA_FLAG_SHORT: return sizeof(short);
    default: return -1;
    }
  case PA_CHAR: return sizeof(char);
  case PA_WCHAR: return sizeof(wchar_t);
  case PA_STRING: return sizeof(char *);
  case PA_WSTRING: return sizeof(wchar_t *);
  case PA_POINTER: return sizeof(void *);
  case PA_FLOAT: return sizeof(float);
  case PA_DOUBLE:
    switch (pa & PA_FLAG_MASK) {
    case 0: return info->alt ? sizeof(float) : sizeof(double);
    case PA_FLAG_LONG_DOUBLE: return sizeof(long double);
    default: return -1;
    }
  default: return -1;
  }

  return -1;
}

static int print_vector_elt(FILE *stream, char *fmt, const struct printf_info *info, int pa, 
                     void *eltp, int i, int width, int prec)
{
  if (pa & PA_FLAG_PTR)
    return fprintf(stream, fmt, *(void **)eltp, i, width, prec);

  switch (pa & (~PA_FLAG_MASK)) {
  case PA_INT:
    switch (pa & PA_FLAG_MASK) {
    case 0:                 return fprintf(stream, fmt, *(int *)       eltp, i, width, prec);
    case PA_FLAG_LONG_LONG: return fprintf(stream, fmt, *(long long *) eltp, i, width, prec);
    case PA_FLAG_LONG:      return fprintf(stream, fmt, *(long *)      eltp, i, width, prec);
    case PA_FLAG_SHORT:     return fprintf(stream, fmt, *(short *)     eltp, i, width, prec);
    default: return -1;
    }
  case PA_CHAR:    return fprintf(stream, fmt, *(char *)     eltp, i, width, prec);
  case PA_WCHAR:   return fprintf(stream, fmt, *(wchar_t *)  eltp, i, width, prec);
  case PA_STRING:  return fprintf(stream, fmt, *(char **)    eltp, i, width, prec);
  case PA_WSTRING: return fprintf(stream, fmt, *(wchar_t **) eltp, i, width, prec);
  case PA_POINTER: return fprintf(stream, fmt, *(void **)    eltp, i, width, prec);

  case PA_FLOAT:   return fprintf(stream, fmt, *(float *)  eltp, i, width, prec);
  case PA_DOUBLE:
    switch (pa & PA_FLAG_MASK) {
    case 0:                   return info->alt ? 
                                     fprintf(stream, fmt, *(float *)        eltp, i, width, prec) :
                                     fprintf(stream, fmt, *(double *)       eltp, i, width, prec);
    case PA_FLAG_LONG_DOUBLE: return fprintf(stream, fmt, *(long double *)  eltp, i, width, prec);
    default: return -1;
    }
  default: return -1;
  }

  return -1;
}

int printf_vector(FILE *stream, 
                  const struct printf_info *info,
                  const void * const *args)
{
  int i;
  int nchars = 0;
  int argtype[4] = { 0, PA_INT, PA_INT, PA_INT };
  int eltsz;

  int nelts;
  char *fmt;
  char *delim;
  void *array;

  fmt = *(char **)args[0];
  delim = *(char **)args[1];
  nelts = *((int*)args[2]);
  array = *(void **)args[3];

  if (parse_printf_format(fmt, 4, argtype) > 4 ||
      argtype[1] != PA_INT || argtype[2] != PA_INT ||
      argtype[3] != PA_INT)
    return -1;
  eltsz = pa2size(info, argtype[0]);

  for (i = 0; i < nelts - 1; i++) {
    nchars += print_vector_elt(stream, fmt, info, argtype[0], array, i, info->width, info->prec);
    nchars += fprintf(stream, delim, i, info->width, info->prec);
    array += eltsz;
  }
  nchars += print_vector_elt(stream, fmt, info, argtype[0], array, i, info->width, info->prec);

  return nchars;
}

int printf_vector_arginfo_size (const struct printf_info *info, 
                                size_t n,
                                int *argtypes,
                                int *size)
{
  if (n >= 4) {
    argtypes[0] = PA_STRING;
    argtypes[1] = PA_STRING;
    argtypes[2] = PA_INT;
    argtypes[3] = PA_POINTER;
    *size = sizeof(void *);
    return 4;
  }
  return 4;
}

