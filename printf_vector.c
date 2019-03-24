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
             ((type (*)(type, void *, void **, void *, int, int, int))(fn))(*(type *)(ptr), fnarg, extra_args, array, i, width, prec) : \
             *(type *)(ptr), \
          extra_args[0],  extra_args[1],  extra_args[2],  extra_args[3], \
          extra_args[4],  extra_args[5],  extra_args[6],  extra_args[7], \
          extra_args[8],  extra_args[9],  extra_args[10], extra_args[11], \
          extra_args[12], extra_args[13], extra_args[14], \
          *(type *)(ptr), \
          i, \
          array, \
          width, \
          prec, \
          fnarg)

static int print_vector_elt(FILE *stream, char *fmt, const struct printf_info *info, int pa,
                            void *extra_args[15],
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

#define MAX_EXTRA_ARGS (15)

static int nextra_args(const struct printf_info *info)
{
  int args = 0;
  if (info->left)
    args += 8;
  if (info->space)
    args += 4;
  if (info->showsign)
    args += 2;
  if (info->group)
    args += 1;
  return args;
}

/* Returns an string explaining the error if there is one, or NULL if it is valid. */
static const char * is_valid_argtype_array(int elt_must_be_pointer, int nextras, int nargs, int argtypes[22])
{
  /* 
     A valid array is of the form
     { elt_type, PA_POINTER, ..., PA_POINTER, elt_type, PA_INT, PA_POINTER, PA_INT, PA_INT, PA_POINTER };
                 |-----MAX_EXTRA_ARGS------|
  */
  /* static const int extra_args_array[MAX_EXTRA_ARGS] = { PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER, */
  /*                                                       PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER, */
  /*                                                       PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER, */
  /*                                                       PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER, }; */
  
  if (nargs < 0)
    return "Per-element format takes a negative number of arguments?!?";
  if (nargs > 22)
    return "Per-element format requires more than the maximum supported number of arguments (22)";
  if (elt_must_be_pointer && argtypes[0] != PA_POINTER)
    return "Per-element format does not expect element to be a pointer, but you specified the '#' option to the 'V' format specififer";
  //if (memcmp(&argtypes[1], extra_args_array, nextras * sizeof(argtypes[0])))
  //  return "Per-element format does not expect all extra args to be pointers";
  if (nargs > MAX_EXTRA_ARGS + 1 && argtypes[MAX_EXTRA_ARGS + 1] != argtypes[0])
    return "The 17th argument must be the same type as the first argument.";
  if (nargs > MAX_EXTRA_ARGS + 2 && (argtypes[MAX_EXTRA_ARGS + 2] & ~PA_FLAG_MASK) != PA_INT)
    return "The 18th argument must be of type int (the index of the current element in the array).";
  if (nargs > MAX_EXTRA_ARGS + 3 && argtypes[MAX_EXTRA_ARGS + 3] != PA_POINTER)
    return "The 19th argument must be a pointer type (the array).";
  if (nargs > MAX_EXTRA_ARGS + 4 && (argtypes[MAX_EXTRA_ARGS + 4] & ~PA_FLAG_MASK) != PA_INT)
    return "The 20th argument must be an int (the number of items in the array).";
  if (nargs > MAX_EXTRA_ARGS + 5 && (argtypes[MAX_EXTRA_ARGS + 5] & ~PA_FLAG_MASK) != PA_INT)
    return "The 21st argument must be an int (the precision arg of the 'V' specifier).";
  if (nargs > MAX_EXTRA_ARGS + 6 && argtypes[MAX_EXTRA_ARGS + 5] != PA_POINTER)
    return "The 22nd argument must be a pointer type (the fnarg optional argument or NULL).";

  return NULL;
}

int printf_vector(FILE *stream, 
                  const struct printf_info *info,
                  const void * const *args)
{
  int i;
  int nchars = 0;
  int nargs;
  int argtype[MAX_EXTRA_ARGS + 7];
  void *extra_args[MAX_EXTRA_ARGS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, };
  int nextras = 0;
  int eltsz;

  int nelts;
  char *eltfmt;
  char *delim;
  void *array;
  void *fn = NULL;
  void *fnarg = NULL;
  int next_arg = 0;
  const char *error_string;
  
  nelts = info->width;
  nextras = nextra_args(info);

  /* Get our args, including optional extra args to be passed through to the per-element format */
  array =   *(void **)args[0];
  eltfmt =  *(char **)args[1];
  delim =   *(char **)args[2];
  if (!info->i18n) {
    next_arg =             3;
  } else {
    fn =    *(void **)args[3];
    fnarg = *(void **)args[4];
    next_arg =             5;
  }
  for (i = 0; i < nextras; i++)
    extra_args[i] = *(void **)args[next_arg++];

  /* Now parse the per-element format and check that it is consistent with the options we were given */
  nargs = parse_printf_format(eltfmt, MAX_EXTRA_ARGS + 7, argtype);
  error_string = is_valid_argtype_array(info->alt, nextras, nargs, argtype);
  if (error_string) {
    return fprintf(stream, "ERRROR: printf_vector: %s", error_string);
  }
  
  /* Compute element size (and possibly override it) from per-element format */
  eltsz = pa2size(info, argtype[0]);
  if (info->prec > 0)
    eltsz = info->prec;

  void * eltp = array;
  for (i = 0; i < nelts - 1; i++) {
    nchars += print_vector_elt(stream, eltfmt, info, argtype[0], extra_args, array, eltp, i, info->width, info->prec, fn, fnarg);
    nchars += print_vector_elt(stream, delim, info, argtype[0], extra_args, array, eltp, i, info->width, info->prec, fn, fnarg);
    eltp += eltsz;
  }
  if (nelts > 0)
    nchars += print_vector_elt(stream, eltfmt, info, argtype[0], extra_args, array, eltp, i, info->width, info->prec, fn, fnarg);

  return nchars;
}

int printf_vector_arginfo_size (const struct printf_info *info, 
                                size_t n,
                                int *argtypes,
                                int *size)
{
  static const int myargtypes[20] = { PA_POINTER, PA_STRING, PA_STRING, // Required
                                      PA_POINTER, PA_POINTER, // 'I' fn and arg
                                      // Optional additional args to per-element and delimiter formats
                                      PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER,
                                      PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER,
                                      PA_POINTER, PA_POINTER, PA_POINTER, PA_POINTER,
                                      PA_POINTER, PA_POINTER, PA_POINTER, 
  };
  int args = 3;
  if (info->i18n)
    args += 2;
  args += nextra_args(info);
  
  memcpy(argtypes, myargtypes, (20 < n ? 20 : n)  * sizeof(argtypes[0]));
  return args;
}

