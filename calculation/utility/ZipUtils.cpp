/**
  Derived from zlib contribution minizip ... see below.

   minizip.c
   Version 1.1, February 14h, 2010
   sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

#if (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#ifdef __APPLE__
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#else
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#include "zip.h"
#include "unzip.h"
#include "FileName.h"
#include "PrjException.h"
#include "ZipUtils.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #include "iowin32.h"
#endif

#include <string>

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt) {
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE) {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#elif (defined __GNUC__ || defined __APPLE__)
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
  int ret=0;
  struct stat s;        // results of stat()
  struct tm* filedate;
  time_t tm_t=0;

  if (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    // strncpy doesnt append the trailing NULL, of the string is too long.
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    // not all systems allow stat'ing a file with / appended
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(const char *f, tm_zip *tmzip, uLong *dt)
{
    return 0;
}
#endif

int check_exist_file(const char* filename) {
    FILE* ftestexist;
    int ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    if (ftestexist==NULL)
        ret = 0;
    else
        fclose(ftestexist);
    return ret;
}

int isLargeFile(const char* filename) {
    int largeFile = 0;
    ZPOS64_T pos = 0;
    FILE* pFile = FOPEN_FUNC(filename, "rb");

    if(pFile != NULL) {
        //int n = FSEEKO_FUNC(pFile, 0, SEEK_END);
        pos = FTELLO_FUNC(pFile);
        //printf("File : %s is %lld bytes\n", filename, pos);
        if(pos >= 0xffffffff)
            largeFile = 1;
        fclose(pFile);
    }
    return largeFile;
}

/** Creates or updates zip archive to add file. */
int addZip(const std::string& filename_try, const std::string& add_filename, bool append) {
    int opt_overwrite=0;
    int opt_compress_level=Z_DEFAULT_COMPRESSION;
    int opt_exclude_path=1;
    int zipok=1;
    int err=0;
    int size_buf= WRITEBUFFERSIZE;
    void* buf=NULL;

    buf = (void*)malloc(size_buf);
    if (buf==NULL) throw memory_exception("Failed to allocate buffer for zip file buffer.");
    opt_overwrite = append ? 2 : 0;
    if (opt_overwrite==2) { // if the file don't exist, we not append file
        if (check_exist_file(filename_try.c_str())==0)
            opt_overwrite=1;
    }
    if (zipok==1) {
        zipFile zf;
        int errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try.c_str(),(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        else
        zf = zipOpen64(filename_try.c_str() ,(opt_overwrite==2) ? 2 : 0);
#        endif
        if (zf == NULL) throw prg_error("Failed to open to file '%s'.", "addZip()", filename_try.c_str());

        FILE * fin;
        int size_read;
        const char* filenameinzip = add_filename.c_str();
        const char *savefilenameinzip;
        zip_fileinfo zi;
        int zip64 = 0;

        zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
        zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
        zi.dosDate = 0;
        zi.internal_fa = 0;
        zi.external_fa = 0;
        filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);
        zip64 = isLargeFile(filenameinzip);
        // The path name saved, should not include a leading slash.
        // if it did, windows/xp and dynazip couldn't read the zip file.
        savefilenameinzip = filenameinzip;
        while( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' ) {
            savefilenameinzip++;
        }
        // should the zip file contain any path at all?
        if( opt_exclude_path ) {
            const char *tmpptr;
            const char *lastslash = 0;
            for( tmpptr = savefilenameinzip; *tmpptr; tmpptr++) {
                if( *tmpptr == '\\' || *tmpptr == '/') {
                    lastslash = tmpptr;
                }
            }
            if( lastslash != NULL ) {
                savefilenameinzip = lastslash+1; // base filename follows last slash.
            }
        }
        err = zipOpenNewFileInZip3_64(
            zf, savefilenameinzip, &zi, NULL, 0, NULL, 0, NULL /* comment*/, (opt_compress_level != 0) ? Z_DEFLATED : 0,
            opt_compress_level, 0, /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
            NULL, 0, zip64
        );
        if (err != ZIP_OK)
            throw prg_error("Error in opening %s in zipfile.", "addZip()", filenameinzip);
        else {
            fin = FOPEN_FUNC(filenameinzip,"rb");
            if (fin == NULL)
                throw prg_error("Error in opening %s for reading.", "addZip()", filenameinzip);
        }

        if (err == ZIP_OK)
            do {
                err = ZIP_OK;
                size_read = (int)fread(buf,1,size_buf,fin);
                if (size_read < size_buf)
                    if (feof(fin) == 0)
                        throw prg_error("Error in reading %s.", "add_zip()", filenameinzip);
                 if (size_read>0) {
                    err = zipWriteInFileInZip (zf,buf,size_read);
                    if (err < 0) throw prg_error("Error in writing %s in the zipfile.", "addZip()", filenameinzip);
                 }
            } while ((err == ZIP_OK) && (size_read > 0));
            if (fin) fclose(fin);
            if (err < 0)
                err = ZIP_ERRNO;
            else {
                err = zipCloseFileInZip(zf);
                if (err != ZIP_OK) throw prg_error("Error in closing %s in the zipfile.", "addZip()", filenameinzip);
            }

            errclose = zipClose(zf,NULL);
            if (errclose != ZIP_OK)
                throw prg_error("Error in closing %s.", "addZip()", filename_try.c_str());
    }
    free(buf);
    return 0;
}

/**
Derived from zlib contribution minizip ... see below.

miniunz.c
Version 1.1, February 14h, 2010
sample part of the MiniZip project - ( http://www.winimage.com/zLibDll/minizip.html )

Copyright (C) 1998-2010 Gilles Vollant (minizip) ( http://www.winimage.com/zLibDll/minizip.html )

Modifications of Unzip for Zip64
Copyright (C) 2007-2008 Even Rouault

Modifications for Zip64 support on both zip and unzip
Copyright (C) 2009-2010 Mathias Svensson ( http://result42.com )
*/

void do_extract_currentfile(unzFile uf, const std::string& extractTo) {
    char filename_inzip[256];
    int err = UNZ_OK;
    FILE *fout = NULL;
    uInt size_buf = WRITEBUFFERSIZE;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
    if (err != UNZ_OK)
        throw prg_error("error %d with zipfile in unzGetCurrentFileInfo\n", "do_extract_currentfile()", err);

    void* buf = (void*)malloc(size_buf);
    if (buf == NULL)
        throw prg_error("Error allocating memory.", "do_extract_currentfile()");
        std::string write_filename(extractTo);
        write_filename += filename_inzip;
        err = unzOpenCurrentFilePassword(uf, 0);
        if (err != UNZ_OK)
            throw prg_error("error %d with zipfile in unzOpenCurrentFilePassword\n", "do_extract_currentfile()", err);
        if (err == UNZ_OK) {
            fout = FOPEN_FUNC(write_filename.c_str(), "wb");
            if (fout == NULL)
                prg_error("error opening %s\n", "do_extract_currentfile()", write_filename.c_str());
        }
        if (fout != NULL) {
            do {
                err = unzReadCurrentFile(uf, buf, size_buf);
                if (err < 0)
                    throw prg_error("error %d with zipfile in unzReadCurrentFile\n", "do_extract_currentfile()", err);
                if (err > 0) {
                    if (fwrite(buf, err, 1, fout) != 1)
                        throw prg_error("error %d in writing extracted file", "do_extract_currentfile()", err);
                }
            } while (err > 0);
            if (fout) fclose(fout);
            //if (err == 0) { change_file_date(write_filename, file_info.dosDate, file_info.tmu_date); }
        }
        if (err == UNZ_OK) {
            err = unzCloseCurrentFile(uf);
            if (err != UNZ_OK) {
                throw prg_error("error %d with zipfile in unzCloseCurrentFile\n", "do_extract_currentfile()", err);
            }
        } else
            unzCloseCurrentFile(uf); // don't lose the error
    free(buf);
}

/** Unzips archive contents for same directory as archive. */
void unZip(const std::string& filename_try) {
    unz_global_info64 gi;
#ifdef USEWIN32IOAPI
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile uf = unzOpen2_64(filename_try.c_str(), &ffunc);
#else
    unzFile uf = unzOpen64(filename_try.c_str());
#endif

    int err = unzGetGlobalInfo64(uf, &gi);
    if (err != UNZ_OK) throw prg_error("error %d with zipfile in unzGetGlobalInfo", "unZip()", err);
    FileName filename(filename_try.c_str());
    std::string extractTo;
    filename.getLocation(extractTo);
    for (uLong i = 0; i < gi.number_entry; ++i) {
        do_extract_currentfile(uf, extractTo);
        if ((i + 1) < gi.number_entry) {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK) throw prg_error("error %d with zipfile in unzGoToNextFile\n", "unZip()", err);
        }
    }
    unzClose(uf);
}
