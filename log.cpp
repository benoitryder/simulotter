#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>

#include "global.h"



/** @brief Compute approximated formatted string length
 *
 * @note This function does not invoke \e va_end. As it invokes \e va_arg, the
 * value of \e ap after the return is indeterminate.
 */
static int len_fmt(const char *fmt, va_list ap)
{
  int len = 0;
  int n;

  while( *fmt != '\0' )
  {
    if( *fmt++ != '%' )
    {
      len++;
      continue;
    }

    if( *fmt == '%' )
    {
      fmt++;
      len++;
      continue;
    }

    // Flags
    while( strchr("'-+ #0", *fmt) != NULL )
      fmt++;

    // Width
    if( *fmt == '*' )
    {
      fmt++;
      n = va_arg(ap, int);
      len += ( n<0 ) ? -n : n;
    }
    else
      len += strtoul(fmt, (char **)&fmt, 10);

    // Precision
    if( *fmt == '.' )
    {
      fmt++;
      if( *fmt == '*' )
      {
        fmt++;
        n = va_arg(ap, int);
        len += ( n<0 ) ? -n : n;
      }
      else
        len += strtoul(fmt, (char **)&fmt, 10);
    }

    // Size modifier
    while( strchr("hlL", *fmt) != NULL )
      fmt++;

    switch( *fmt )
    {
      case 'd': case 'i': case 'o': case 'u': case 'x': case 'X':
        va_arg(ap, int);
        // Max integer size + sign (hint: 2^64 has 20 characters)
        len += 25;
        break;
      case 'f':
        // Max 64b float: ~10^308 (+ sign + ...)
        va_arg(ap, double);
        len += 320;
        break;
      case 'e': case 'E': case 'g': case 'G':
        va_arg(ap, double);
        // Max 64bit exposant: 308 (+ ...)
        len += 20;
        break;
      case 'c': case 'C':
        va_arg(ap, int);
        len += 1;
        break;
      case 's': case 'S':
        // + 10, to write "(null)" or something for null pointers
        len += strlen( va_arg(ap, char*) ) + 10;
        break;
      case 'p':
        va_arg(ap, void*);
        // Converted in "an implementation-dependant manner"
        len += 25;
        break;
      case 'n':
        va_arg(ap, int*);
        break;
    }
  }

  return len+1;
}


int ssprintf(char **ptr, const char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vssprintf(ptr, fmt, ap);
  va_end(ap);

  return ret;
}

int vssprintf(char **ptr, const char *fmt, va_list ap)
{
  va_list ap2;
  int len;
  int ret;

  va_copy(ap2, ap);
  len = len_fmt(fmt, ap2);
  va_end(ap2);

  *ptr = (char *)malloc(len);
  if( *ptr == NULL )
    return -1;

  ret = vsnprintf(*ptr, len, fmt, ap);
  (*ptr)[len-1] = '\0';
  va_end(ap);

  return ret;
}


void Log::trace(const char *fmt, ...)
{
  va_list ap;
  char *s;

  va_start(ap, fmt);
  vssprintf(&s, fmt, ap);
  va_end(ap);

  fprintf(stdout, "%s\n", s);
  if( cfg != NULL && cfg->log_flush )
    fflush(stdout);
  free(s);
}



Error::Error(const char *fmt, ...): msg(NULL)
{
  va_list ap;
  va_start(ap, fmt);
  setMsg(fmt, ap);
  va_end(ap);
}


Error::Error(const Error &e): std::exception(e)
{
  operator=(e);
}

Error &Error::operator=(const Error &e)
{
  if( e.msg == NULL )
    this->msg = NULL;
  else
  {
    int n = strlen(e.msg);
    this->msg = (char *)malloc(n+1);
    strncpy(this->msg, e.msg, n);
    this->msg[n] = '\0';
  }
  return *this;
}

void Error::setMsg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  setMsg(fmt, ap);
  va_end(ap);
}

void Error::setMsg(const char *fmt, va_list ap)
{
  free(msg);
  vssprintf(&msg, fmt, ap);
}

