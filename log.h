#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <exception>
#include <malloc.h>

///@file


/** @brief Allocate a formatted string
 *
 * The returned string should be freed using \e free.
 * If the function fails <tt>ptr</tt> points to the null pointer.
 *
 * @return The number of characters written (not including the trailing NUL) or
 * a negative number on error.
 *
 * @note This function is equivalent to the non POSIX asprintf function.
 */
int ssprintf(char **ptr, const char *fmt, ...);

/// Allocate a formatted string, \e va_list version
int vssprintf(char **ptr, const char *fmt, va_list ap);


class Log
{
public:
  Log() { trace("log is running"); }
  ~Log() {}

  void trace(const char *fmt, ...);
};


class Error: public std::exception
{
public:

  Error(const char *fmt, ...);
  Error() { this->msg = NULL; }
  ~Error() throw() { free(msg); }
  Error(const Error &e);
  Error &operator=(const Error &e);

  virtual const char *what() const throw()
  {
    return this->msg;
  }

private:
  char *msg;
};


#endif
