#ifndef LOG_H_
#define LOG_H_

///@file

#include <cstdarg>
#include <exception>

/// Logging macro, for convenience.
#define LOG (::Logger::glog)


class Logger
{
public:
  Logger() { log("log is running"); }
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
  Error(): msg(NULL) {}
  ~Error() throw();
  Error(const Error &e);
  Error &operator=(const Error &e);

  virtual const char *what() const throw()
  {
    return this->msg;
  }

protected:
  void setMsg(const char *fmt, ...);
  void setMsg(const char *fmt, va_list ap);

private:
  char *msg;
};


#endif
