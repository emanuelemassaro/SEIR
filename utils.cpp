#ifdef WIN32
#  include <windows.h>
#  include "shlwapi.h"
#else
#  include <string.h>
#  include <unistd.h>
#  include <sys/stat.h>
#endif

#include <vector>
#include <algorithm>

#include "utils.h"

using namespace std;

bool ParseListOfLongsFromCsv(char *str, vector<long> &out)
{
  out.clear();

  char buffer[128];
  int bufIx = 0;

  char *ptr = str;
  bool isInQuote = false;
  while (*ptr && *ptr != '\n')
  {
    switch (*ptr)
    {
      case ('\"'):
      {
        isInQuote = !isInQuote;
        break;
      }
      case (','):
      {
        if (!isInQuote)
        {
          buffer[bufIx++] = 0;

          long tmp;
          if (sscanf(buffer, "%ld", &tmp) != 1)
            return false;
          out.push_back(tmp);
          bufIx = 0;
          break;
        }
      }
      // FALLTROUGH INTENDED
      default:
      {
        buffer[bufIx++] = *ptr;
        break;
      }
    }
    ptr ++;
  }
  buffer[bufIx++] = 0;
  long tmp;
  if (sscanf(buffer, "%ld", &tmp) != 1)
    return false;
  out.push_back(tmp);
  return true;
}

#ifdef WIN32
  static OSVERSIONINFO *WinVersion()
  {
    static OSVERSIONINFO osvi;
    static bool done = false;
    if (!done)
    {
      ::ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
      osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if(!::GetVersionEx(&osvi))
        return 0;
      done = true;
    }
    return &osvi;
  }
#endif

namespace FS
{
  bool isPathAbsolute(const char *path)
  {
#ifdef WIN32
    return path && strchr(path, ':');
#else
    return path && *path == '/';
#endif
  }

  bool fsEntryExist(const char *path)
  {
#ifdef WIN32
    OSVERSIONINFO *osvi = WinVersion();
    if (osvi && osvi->dwMajorVersion >= 5)
      return PathFileExistsA(path) == TRUE;
    else
      return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat buffer;
    return !stat(path, &buffer);
#endif
  }

#ifdef WIN32
  bool makeDir(const char *rootDir, const char *dirPath)
#else
  bool makeDir(const char *rootDir, const char *dirPath, int imode)
#endif
  {
    if (!rootDir || !*rootDir || !isPathAbsolute(rootDir) ||
        !dirPath || !*dirPath)
      return false;

#ifndef WIN32
    mode_t mode = imode == -1 ? S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH : (mode_t) imode;

    char originalCWD[MAXPATHLENGTH];
    if (!getcwd(originalCWD, MAXPATHLENGTH))
      return false;

    char path[MAXPATHLENGTH];
    strcpy(path, dirPath);
    
    if (chdir(rootDir))
      return false;

    int lastPS = -1;
    for (int i = 0; ; i++)
    {
      if (isPathSep(path[i]) || !path[i])
      {
        if (i - lastPS > 1)
        {
          char tmp = path[i];
          path[i] = 0;
          char *ptr = path + lastPS + 1;
          if (chdir(ptr) &&
              (mkdir(ptr, mode) ||
               chdir(ptr)))
          {
            chdir(originalCWD);
            return false;
          }
          path[i] = tmp;
        }
        lastPS = i;
        if (!path[i])
          break;
      }
    }
    chdir(originalCWD);
#else
    const char *path = dirPath;

    char p[MAXPATHLENGTH];
    strcpy(p, rootDir);

    int len = strlen(p);
    if (!isPathSep(p[len - 1]))
      p[len++] = pathSep();

    for (int i = 0; ; i++)
    {
      if (isPathSep(path[i]) || !path[i])
      {
        if (isPathSep(p[len - 1]))
        {
          if (!path[i])
            break;
          else
            continue;
        }
        p[len] = 0;
        if (!CreateDirectoryA(p, 0) && GetLastError() != ERROR_ALREADY_EXISTS)
          return false;

        p[len++] = pathSep();
        if (!path[i])
          break;
      }
      else
        p[len++] = path[i];
    }
#endif

    return true;
  }
}