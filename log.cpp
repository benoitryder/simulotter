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



Error::Error(const char *fmt, ...): msg_(NULL)
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
  delete msg_;
}

Error &Error::operator=(const Error &e)
{
  if( e.msg_ == NULL )
    msg_ = NULL;
  else
  {
    delete[] msg_;
    msg_ = NULL;
    int n = ::strlen(e.msg_);
    msg_ = new char[n+1];
    ::strncpy(msg_, e.msg_, n);
    msg_[n] = '\0';
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
  delete[] msg_;
  msg_ = NULL;

  // build the message string
  va_list ap2;
  va_copy(ap2,ap);
  int n = ::vsnprintf(NULL, 0, fmt, ap2) + 1;
  va_end(ap2);
  msg_ = new char[n];
  ::vsnprintf(msg_, n, fmt, ap);
}

