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
    case 0: return info->is_short ? sizeof(float) : sizeof(double);
    case PA_FLAG_LONG_DOUBLE: return sizeof(long double);
    default: return -1;
    }
  default: return -1;
  }

  return -1;
}

#define FPRINTF_AS_TYPE(type, ptr) \
  fprintf(stream, \
          fmt, \
          (fn) ? \
             ((type (*)(type, void *, void *, int, int, int))(fn))(*(type *)(ptr), fnarg, array, i, width, prec) : \
             *(type *)(ptr), \
          *(type *)(ptr), \
          i, \
          array, \
          width, \
          prec)

static int print_vector_elt(FILE *stream, char *fmt, const struct printf_info *info, int pa,
                            void *array, void *eltp, int i, int width, int prec, void *fn, void *fnarg)
{
  if (pa & PA_FLAG_PTR) {
    if (info->alt)
      return FPRINTF_AS_TYPE(void *, &eltp);
    else 
      return FPRINTF_AS_TYPE(void *, eltp);
  }
  
  switch (pa & (~PA_FLAG_MASK)) {
  case PA_INT:
    switch (pa & PA_FLAG_MASK) {
    case 0:                 return FPRINTF_AS_TYPE(int,       eltp);
    case PA_FLAG_LONG_LONG: return FPRINTF_AS_TYPE(long long, eltp);
    case PA_FLAG_LONG:      return FPRINTF_AS_TYPE(long,      eltp);
    case PA_FLAG_SHORT:     return FPRINTF_AS_TYPE(short,     eltp);
    default: return -1;
    }
  case PA_CHAR:    return FPRINTF_AS_TYPE(char,      eltp);
  case PA_WCHAR:   return FPRINTF_AS_TYPE(wchar_t,   eltp);

  case PA_STRING:  return FPRINTF_AS_TYPE(char *,    eltp);
  case PA_WSTRING: return FPRINTF_AS_TYPE(wchar_t *, eltp);
  case PA_POINTER:
    if (info->alt)
      return FPRINTF_AS_TYPE(void *, &eltp);
    else 
      return FPRINTF_AS_TYPE(void *, eltp);

  case PA_FLOAT:   return FPRINTF_AS_TYPE(float, eltp);
  case PA_DOUBLE:
    switch (pa & PA_FLAG_MASK) {
    case 0:                   return info->is_short ? 
                                     FPRINTF_AS_TYPE(float,       eltp) :
                                     FPRINTF_AS_TYPE(double,      eltp);
    case PA_FLAG_LONG_DOUBLE: return FPRINTF_AS_TYPE(long double, eltp);
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
  int nargs;
  int argtype[6] = { 0, 0, PA_INT, PA_POINTER, PA_INT, PA_INT };
  int eltsz;

  int nelts;
  char *eltfmt;
  char *delim;
  void *array;
  void *fn = NULL;
  void *fnarg = NULL;
  
  nelts = info->width;
  array = *(void **)args[0];
  eltfmt = *(char **)args[1];
  delim = *(char **)args[2];
  if (info->showsign) {
    fn = *(void **)args[3];
    fnarg = *(void **)args[4];
  }
  
  nargs = parse_printf_format(eltfmt, 4, argtype);
  if (nargs < 0 || nargs > 6 ||
      (nargs > 1 && argtype[0] != argtype[1]) || 
      argtype[2] != PA_INT ||
      argtype[3] != PA_POINTER ||
      argtype[4] != PA_INT ||
      argtype[5] != PA_INT)
    return -1;
  eltsz = pa2size(info, argtype[0]);
  if (info->prec > 0)
    eltsz = info->prec;
  if (info->alt && argtype[0] != PA_POINTER)
    return -1;

  void * eltp = array;
  for (i = 0; i < nelts - 1; i++) {
    nchars += print_vector_elt(stream, eltfmt, info, argtype[0], array, eltp, i, info->width, info->prec, fn, fnarg);
    nchars += fprintf(stream, delim, i, array, info->width, info->prec);
    eltp += eltsz;
  }
  if (nelts > 0)
    nchars += print_vector_elt(stream, eltfmt, info, argtype[0], array, eltp, i, info->width, info->prec, fn, fnarg);

  return nchars;
}

int printf_vector_arginfo_size (const struct printf_info *info, 
                                size_t n,
                                int *argtypes,
                                int *size)
{
  static const int myargtypes[5] = { PA_POINTER, PA_STRING, PA_STRING, PA_POINTER, PA_POINTER };
  memcpy(argtypes, myargtypes, (5 < n ? 5 : n)  * sizeof(argtypes));
  if (info->showsign)
    return 5;
  else 
    return 3;
}

