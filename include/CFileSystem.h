#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>

#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif

/* You only need to format LITTLEFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */
   
#define FORMAT_LITTLEFS_IF_FAILED true

class CFileSystem
{
    

//BASIC SINGLETON START
public:
    static CFileSystem &getInstance()
    {
        static CFileSystem S;
        return S;
    }


    CFileSystem(const CFileSystem &) = delete;
    void operator=(CFileSystem const &) = delete;

public:
    CFileSystem();
    ~CFileSystem();

private:
    fs::FS* m_pFS;

//BASIC SINGLETON END
public:
    void listDir    (const char * _dirname, uint8_t levels);
    void createDir  (const char * _path);
    void removeDir  (const char * _path);
    void writeFile  (const char * _path, const char * _message);
    void appendFile (const char * _path, const char * _message);
    void readFile   (const char * _path);
    bool readFile(const char *path, String* _pDestinationString);
    void deleteFile (const char * _path);
    void renameFile (const char * _path1, const char * _path2);
    void testFileIO (const char * _path);
    void deleteFile2(const char * _path);
    void writeFile2 (const char * _path, const char *_message);

    void writeFile(const char *path, String *_pString);
};






