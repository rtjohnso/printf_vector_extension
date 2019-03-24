/* Copyright 2019 Rob Johnson <rtjohnsocs42@gmail.com> */
/* BSD License. */

#include <stdio.h>
#include <printf_vector.h>

int main(int argc, char **argv)
{
  char * S[3] = { "one", "two", "three" };
  int A[3] = { 96, 97, 98 };
  short B[3] = { 45, 46, 47 };
  float F[3] = { 3.1415926535897, 2.718281828459, 1.77245385090 };
  long double LD[3] = { -3.1415926535897, -2.718281828459, -1.77245385090 };


  register_printf_specifier('V', printf_vector, printf_vector_arginfo_size);

  /* Some simple examples. */

  /*     <Overall template>       [width] [precision]    <per-element template>   <delimiter template>    <nelements>   <vector> */
  printf("{ %V }\n",                                     "%s",                    ", ",                   3,            S);
  printf("{ %V }\n",                                     "%Lf",                   ", ",                   3,            LD);
  printf("Hex dump of F: %V\n",                          "%02hhx",                "",                     sizeof(F),    F);

  /* The index of the current item being printed is available as the
     second argument to the per-element format string. */
  printf("{\n%V\n}\n",                                   "  A[%2$d] = %d;",       "\n",                   3,            A);

  /* The width modifier of the %V conversion is passed through as the
     third argument to the per-element format string. */
  printf("{ %*V }\n",             4,                     "%0*3$hx",               ", ",                   3,            B);

  /* For technical reasons, you have to specify the # flag to %V when
     printing floats  (doubles and long doubles do not require #). */
  printf("{ %#V }\n",                                    "%f",                    ", ",                   3,            F);

  /* The precision modifier is passed through as argument 4 of the
     per-element format string. */
  printf("{ %#*.*V }\n",          8,      4,             "%-*3$.*4$f",            ", ",                   3,            F);

  /* The index, width, and precision are also pased as arguments to
     the delimiter, just in case you'd ever want them. */
  printf("{\n%*.*V\n}\n",         20,     4,             "%*3$.*4$Lf",            "\n   (%d %d %d)\n",    3,            LD);

  return 0;
}
