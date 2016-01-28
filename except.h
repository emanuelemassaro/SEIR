#ifndef EXCEPT_HEADER_FILE
#define EXCEPT_HEADER_FILE

#include <string.h>

class Exception
{
  char *m_msg;
public:
  Exception(const char *msg)
  {
    int len = strlen(msg);
    m_msg = new char [len + 1];
    strcpy(m_msg, msg);
  }
  virtual ~Exception()
  {
    delete [] m_msg;
  }

  inline const char *message() const { return m_msg; }
};

class SystemException : public Exception
{
protected:
  int m_sysError;
public:
  SystemException(const char *v, int errCode) :
    Exception(v)
  {
    m_sysError = errCode;
  }

  inline int sysError() const { return m_sysError; }
};

#endif // EXCEPT_HEADER_FILE
