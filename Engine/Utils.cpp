// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-

#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>

#if defined(WINDOWS)
#include <windows.h>
#endif


char* FileGetAsText(const char* fname)
{
    char* ret = nullptr;
    
    struct stat buf;
    if (stat(fname, &buf) != -1)
    {
        FILE* fh = fopen(fname, "rb");
        ret = new char[buf.st_size+1];
        fread(ret, 1, buf.st_size, fh);
        fclose(fh);
        
        ret[buf.st_size] = '\0';
    }
    
    return ret;
}

int Printf(const char* fmt, ...)
{
  int ret = 0;
  va_list args;
  va_start(args, fmt);

  char buf[2048];
  ret = vsnprintf(buf, sizeof buf, fmt, args);
  fputs(buf, stdout);

#if defined(WINDOWS)
  OutputDebugStringA(buf);
#endif
  va_end(args);

  return ret;
}

int FPrintf(FILE* fh, const char* fmt, ...)
{
  int ret = 0;
  va_list args;
  va_start(args, fmt);

#if defined(WINDOWS)
  if (fh == stderr)
  {
    char buf[2048];
    vsnprintf(buf, sizeof buf, fmt, args);
    
    OutputDebugStringA(buf);
  }
  else
#endif
  {
    ret = vfprintf(fh, fmt, args);
  }
  
  va_end(args);

  return ret;
}
