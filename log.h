#ifndef LOG_H_
#define LOG_H_

///@file

#include <cstdarg>
#include <exception>
#include <string>

/// Logging macro, for convenience.
#define LOG (::Logger::glog)


/// printf-like formatting for std::string.
std::string stringf(const char *fmt, ...);
/// printf-like formatting for std::string, variadic version.
std::string vstringf(const char *fmt, va_list ap);


class Logger
{
public:
  Logger() {}
  ~Logger() {}

  void log(const char *fmt, ...);
  void vlog(const char *fmt, va_list ap);

  /// Global logging method.
  static void glog(const char *fmt, ...);

private:
  /// Global logger instance.
  static Logger logger_;
};


class Error: public std::exception
{
public:
  Error(const char *fmt, ...);
  Error(const std::string &msg): what_(msg) {}
  ~Error() throw() {}

  virtual const char *what() const throw()
  {
    return what_.c_str();
  }

private:
  std::string what_;
};


#endif
