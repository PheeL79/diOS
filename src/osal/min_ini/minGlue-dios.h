/*  Glue functions for the minIni library, based on the FatFs and Petit-FatFs
 *  libraries, see http://elm-chan.org/fsw/ff/00index_e.html
 *
 *  By CompuPhase, 2008-2012
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 *
 *  (The FatFs and Petit-FatFs libraries are copyright by ChaN and licensed at
 *  its own terms.)
 */

#include <string.h>
#include "os_settings.h"
     
//#define INI_READONLY
#if (1 != OS_SETTINGS_BROWSE_ENABLED)
#define INI_NOBROWSE
#endif // OS_SETTINGS_BROWSE_ENABLED
#define INI_ANSIONLY                                                /* ignore UNICODE or _UNICODE macros, compile as ASCII/ANSI */
#define PORTABLE_STRNICMP
#define INI_BUFFERSIZE                  OS_SETTINGS_BUFFER_LEN      /* maximum line length, maximum path length */
//#define INI_LINETERM                  "\r\n"

#if defined(CM4F)
#define INI_REAL FLT
#define ini_ftoa(string,value) sprintf((string),"%f",(value))
#define ini_atof(string) (INI_REAL)strtod((string),NULL)
#endif // CM4F

#define INI_FILETYPE                    OS_FileHd
#define ini_openread(filename,file)     (OS_FileOpen((file), (ConstStrPtr)(filename),\
                                         (OS_FileOpenMode)(BIT(OS_FS_FILE_OP_MODE_OPEN_EXISTS) | BIT(OS_FS_FILE_OP_MODE_READ))) == S_OK)
#define ini_openwrite(filename,file)    (OS_FileOpen((file), (ConstStrPtr)(filename),\
                                         (OS_FileOpenMode)(BIT(OS_FS_FILE_OP_MODE_OPEN_EXISTS) | BIT(OS_FS_FILE_OP_MODE_OPEN_NEW) |\
                                          BIT(OS_FS_FILE_OP_MODE_WRITE))) == S_OK)
#define ini_close(file)                 (OS_FileClose(file) == S_OK)
#define ini_read(buffer,size,file)      (OS_FileGetS(*(file), (StrPtr)(buffer), (size)) == S_OK)
#define ini_write(buffer,file)          (OS_FilePutS(*(file), (StrPtr)(buffer)) == S_OK)
#define ini_remove(filename)            (OS_FileDelete(filename) == S_OK)

#define INI_FILEPOS                     U32
#define ini_tell(file,pos)              (*(pos) = OS_FileTell(*(file)))
#define ini_seek(file,pos)              (OS_FileLSeek(*(file), *(pos)) == S_OK)

static int ini_rename(TCHAR *source, const TCHAR *dest)
{
  /* Function f_rename() does not allow drive letters in the destination file */
  char *drive = strchr(dest, ':');
  drive = (drive == NULL) ? (char *)dest : drive + 1;
  return (OS_FileRename((ConstStrPtr)source, (ConstStrPtr)drive) == S_OK);
}
