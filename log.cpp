#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include "log.h"



std::string stringf(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  std::string ret = vstringf(fmt, ap);
  va_end(ap);
  return ret;
}

std::string vstringf(const char* fmt, va_list ap)
{
  va_list ap2;
  va_copy(ap2,ap);
  int n = vsnprintf(NULL, 0, fmt, ap2) + 1;
  va_end(ap2);
  char msg[n];
  vsnprintf(msg, n, fmt, ap);
  return std::string(msg);
}


void Logger::log(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vlog(fmt, ap);
  va_end(ap);
}

void Logger::vlog(const char* fmt, va_list ap)
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

void Logger::glog(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  logger_.vlog(fmt, ap);
  va_end(ap);
}

Logger Logger::logger_;



Error::Error(const char* fmt, ...): std::exception()
{
  va_list ap;
  va_start(ap, fmt);
  what_ = vstringf(fmt, ap);
  va_end(ap);
}

