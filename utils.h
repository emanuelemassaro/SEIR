#ifndef UTILS_HEADER_FILE_INCLUDED
#define UTILS_HEADER_FILE_INCLUDED

#include <stdio.h>
#include <vector>
#include "stepdata.h"

#define MAXPATHLENGTH 4096

bool ParseListOfLongsFromCsv(char *str, std::vector<long> &out);

namespace FS
{
  bool isPathAbsolute(const char *path);
  bool fsEntryExist(const char *name);
#ifdef WIN32
  bool makeDir(const char *rootDir, const char *dirPath);
#else
  bool makeDir(const char *rootDir, const char *dirPath, int imode = -1);
#endif

  inline bool isPathSep(int sym)
  {
#ifdef WIN32
    return sym == '\\' || sym == '/';
#else
    return sym == '/';
#endif
  }

  inline char pathSep()
  {
#ifdef WIN32
    return '\\';
#else
    return '/';
#endif
  }
};

class auto_file
{
  FILE *m_file;

public:
  auto_file(FILE *f)
  {
    m_file = f;
  }
  virtual ~auto_file()
  {
    if (m_file)
      fclose(m_file);
    m_file = 0;
  }
};

template<class T>
  class auto_del
  {
    T *m_ptr;
    bool m_isarr;

  public:
    auto_del(T *ptr, bool isarr) :
      m_isarr(isarr)
    {
      m_ptr = ptr;
    }
    virtual ~auto_del()
    {
      if (m_ptr)
        if (m_isarr)
          delete [] m_ptr;
        else
          delete m_ptr;
      m_ptr = 0;
    }
  };

#endif // UTILS_HEADER_FILE_INCLUDED
