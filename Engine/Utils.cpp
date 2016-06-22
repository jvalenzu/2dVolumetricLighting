#include <stdio.h>
#include <sys/stat.h>

char* FileGetAsText(const char* fname)
{
    char* ret = NULL;
    
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
