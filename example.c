/* Copyright 2019 Rob Johnson <rtjohnsocs42@gmail.com> */
/* BSD License. */

#include <arpa/inet.h>
#include <stdio.h>
#include <math.h>
#include <printf_vector.h>

/*
 * The widget example is just to show how to use the vector code to
 * print out other user-defined types. 
 */
typedef struct widget {
  char * name;
  int value;
} widget;

int printf_widget(FILE *stream, 
                  const struct printf_info *info,
                  const void * const *args)
{
  widget *w = *(widget **)args[0];
  return fprintf(stream, "Widget(%s, %d)", w->name, w->value);
}

int printf_widget_arginfo_size (const struct printf_info *info, 
                                size_t n,
                                int *argtypes,
                                int *size)
{
  if (n >= 1) {
    argtypes[0] = PA_POINTER;
    *size = sizeof(void *);
  }
  return 1;
}

int main(int argc, char **argv)
{
  char * S[3] = { "one", "two", "three" };
  int A[3] = { 96, 97, 98 };
  short B[3] = { 45, 46, 47 };
  float F[3] = { 3.1415926535897, 2.718281828459, 1.77245385090 };
  long double LD[3] = { -3.1415926535897, -2.718281828459, -1.77245385090 };
  widget W[3] = { { "foo", 1 }, { "bar", 2 }, { "baz", 3 } };
  widget *WP[4] = { &W[0], &W[2], &W[1], &W[0] };
  
  register_printf_specifier('V', printf_vector, printf_vector_arginfo_size);
  register_printf_specifier('W', printf_widget, printf_widget_arginfo_size);

  /* The number of items in your vector is specified as the width option to
     the 'V' specifier, e.g. "%3V" specifies a vector of 3 elements.
     Use the '*' specifier if you need to specify a run-time computed
     length. */
  
  /* The printf_vector code attempts to infer the size of elements of
     your array from the per-element format string, so you need to
     make sure you specify the right long/short/char specifier and
     modifiers for integer types.  For floating point types, you need
     to specify 'L' for long doubles.  For floats, you need to specify
     'h' as a modifier to the 'V' specifier (an extension specific to
     the printf_vector code). */
  
  /* The index of the current item being printed is available as the
     third argument to the per-element format string. */

  /* If you specify the '#' modifier to the 'V' specifier, then a
     pointer to the ith element, rather than the ith element itself,
     will be passed to the per-element template. */

  /* If you specify a precision w for the 'V' conversion, then it will
     treat each element of the vector as being of size w bytes. This
     can be combined with the '#' modifier to pass a pointer to each
     element of an array of structs, and using a custom specifier to
     print out each struct, as in the widget example below.
  */

  /* If you specify the '+' modifier, then you can pass a function
     (and auxiliary argument) that will be applied to each element of
     the vector before printing it. The function will be called as
     fn(elt, arg, indx, array, width, prec), where indx is the indx of
     the current element, array is the vector being printed, width and
     prec are the width and precision options to the 'V' specifier.
     This can be used to do, say, endianness conversion. The original
     value is the second argument to the per-element format. */
  
  /* Some simple examples. */

  /*     <Overall template>           [nelts]      [eltsize]           <vector>   <per-element template>   <delimiter template>   [func]   [arg] */
  printf("S  = { %3V }\n",                                             S,         "%s",                    ", "                                      );
  printf("LD = { %*V }\n",            3,                               LD,        "%Lf",                   ", "                                      );
  printf("B  = { %3V }\n",                                             B,         "%hd",                   ", "                                      );
  printf("A  = {\n%3V\n}\n",                                           A,         "  A[%3$d] = %d;",       "\n"                                      );
  printf("WP = { %3V }\n",                                             WP,        "%W",                    ", "                                      );
  printf("W  = { %#3.*V }\n",                      sizeof(W[0]),       W,         "%W",                    ", "                                      );
  printf("F  = { %3hV }\n",                                            F,         "%f",                    ", "                                      );
  printf("    Hex dump of F: %*V\n",  sizeof(F),                       F,         "%02hhx",                ""                                        );
  printf("Another dump of F: %+*V\n", sizeof(F)/2,                     F,         "%04hx",                 " ",                   ntohs,   NULL      );
  printf("sqrt(F)  = { %+3hV }\n",                                     F,         "%f",                    ", ",                  sqrtf,   NULL      );

  printf("Example: \n%+3hV\n",                                           F,         "  sqrt(F[%3$d]) = sqrt(%2$f) = %f",                    "\n",                  sqrtf,  NULL   );

  return 0;
}
