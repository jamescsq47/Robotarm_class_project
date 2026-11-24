/* C runtime constructors; data/bss initialization is performed by startup. */
typedef void (*init_fn)(void);
extern init_fn __preinit_array_start[], __preinit_array_end[];
extern init_fn __init_array_start[], __init_array_end[];
void __libc_init_array(void)
{
  for(init_fn *f=__preinit_array_start; f<__preinit_array_end; ++f) (*f)();
  for(init_fn *f=__init_array_start; f<__init_array_end; ++f) (*f)();
}

#include <stddef.h>
void *memset(void *dest, int value, size_t size)
{
  volatile unsigned char *out = (volatile unsigned char *)dest;
  while(size--) *out++ = (unsigned char)value;
  return dest;
}
