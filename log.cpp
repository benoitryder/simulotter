#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include "config.h"
#include "log.h"



void Logger::log(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  this->vlog(fmt, ap);
  va_end(ap);
}

void Logger::vlog(const char *fmt, va_list ap)
{
  // build the message string
  va_list ap2;
  va_copy(ap2,ap);
  int n = vsnprintf(NULL, 0, fmt, ap2) + 1;
  va_end(ap2);
  char msg[n];
  vsnprintf(msg, n, fmt, ap);

  fprintf(stdout, "%s\n", msg);
  if( cfg.log_flush )
    fflush(stdout);
}

void Logger::glog(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  logger_.vlog(fmt, ap);
  va_end(ap);
}

Logger Logger::logger_;



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

Error::~Error() throw()
{
  delete this->msg;
}

Error &Error::operator=(const Error &e)
{
  if( e.msg == NULL )
    this->msg = NULL;
  else
  {
    delete[] this->msg;
    this->msg = NULL;
    int n = ::strlen(e.msg);
    this->msg = new char[n+1];
    ::strncpy(this->msg, e.msg, n);
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
  delete[] this->msg;
  msg = NULL;

  // build the message string
  va_list ap2;
  va_copy(ap2,ap);
  int n = ::vsnprintf(NULL, 0, fmt, ap2) + 1;
  va_end(ap2);
  this->msg = new char[n];
  ::vsnprintf(this->msg, n, fmt, ap);
}

