
#include "CFileSystem.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>


CFileSystem::CFileSystem()
{


    this->m_pFS = &LittleFS;

    if (!this->m_pFS->begin())
    {
        Serial.println("LITTLEFS Mount Failed");
        return;
    }
}

CFileSystem::~CFileSystem()
{
}


void CFileSystem::listDir(const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = this->m_pFS->open(dirname, "r");
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.name());
#else
            Serial.print(file.name());
            time_t t = file.getLastWrite();
            struct tm *tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
#endif

            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");

#ifdef CONFIG_LITTLEFS_FOR_IDF_3_2
            Serial.println(file.size());
#else
            Serial.print(file.size());
            time_t t = file.getLastWrite();
            struct tm *tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
#endif
        }
        file = root.openNextFile();
    }
}

void CFileSystem::createDir(const char *path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if (this->m_pFS->mkdir(path))
    {
        Serial.println("Dir created");
    }
    else
    {
        Serial.println("mkdir failed");
    }
}

void CFileSystem::removeDir(const char *path)
{
    Serial.printf("Removing Dir: %s\n", path);
    if (this->m_pFS->rmdir(path))
    {
        Serial.println("Dir removed");
    }
    else
    {
        Serial.println("rmdir failed");
    }
}

void CFileSystem::readFile(const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = this->m_pFS->open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while (file.available())
    {
        Serial.write(file.read());
    }
    file.close();
}

bool CFileSystem::readFile(const char *path, String* _pDestinationString)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = this->m_pFS->open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        _pDestinationString->clear();
        return false;
    }

    Serial.print("- read from file:");
    
    
    while (file.available())
    {
        *_pDestinationString = file.readString();
        Serial.write(_pDestinationString->c_str());
    }


    file.close();

    return true;
}




void CFileSystem::writeFile(const char *path, const char *message)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = this->m_pFS->open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}


void CFileSystem::writeFile(const char *path, String *_pString)
{
    Serial.printf("Writing file: %s\r\n", path);

    File file = this->m_pFS->open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(*_pString))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}

void CFileSystem::appendFile(const char *path, const char *message)
{
    Serial.printf("Appending to file: %s\r\n", path);

    File file = this->m_pFS->open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for appending");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- message appended");
    }
    else
    {
        Serial.println("- append failed");
    }
    file.close();
}

void CFileSystem::renameFile(const char *path1, const char *path2)
{
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (this->m_pFS->rename(path1, path2))
    {
        Serial.println("- file renamed");
    }
    else
    {
        Serial.println("- rename failed");
    }
}

void CFileSystem::deleteFile(const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if (this->m_pFS->remove(path))
    {
        Serial.println("- file deleted");
    }
    else
    {
        Serial.println("- delete failed");
    }
}

// SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

void CFileSystem::writeFile2(const char *path, const char *message)
{
    if (!this->m_pFS->exists(path))
    {
        if (strchr(path, '/'))
        {
            Serial.printf("Create missing folders of: %s\r\n", path);
            char *pathStr = strdup(path);
            if (pathStr)
            {
                char *ptr = strchr(pathStr, '/');
                while (ptr)
                {
                    *ptr = 0;
                    this->m_pFS->mkdir(pathStr);
                    *ptr = '/';
                    ptr = strchr(ptr + 1, '/');
                }
            }
            free(pathStr);
        }
    }

    Serial.printf("Writing file to: %s\r\n", path);
    File file = this->m_pFS->open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file written");
    }
    else
    {
        Serial.println("- write failed");
    }
    file.close();
}

void CFileSystem::deleteFile2(const char *path)
{
    Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

    if (this->m_pFS->remove(path))
    {
        Serial.println("- file deleted");
    }
    else
    {
        Serial.println("- delete failed");
    }

    char *pathStr = strdup(path);
    if (pathStr)
    {
        char *ptr = strrchr(pathStr, '/');
        if (ptr)
        {
            Serial.printf("Removing all empty folders on path: %s\r\n", path);
        }
        while (ptr)
        {
            *ptr = 0;
            this->m_pFS->rmdir(pathStr);
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
}

void CFileSystem::testFileIO(const char *path)
{
    Serial.printf("Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = this->m_pFS->open(path, "w");
    if (!file)
    {
        Serial.println("- failed to open file for writing");
        return;
    }

    size_t i;
    Serial.print("- writing");
    uint32_t start = millis();
    for (i = 0; i < 2048; i++)
    {
        if ((i & 0x001F) == 0x001F)
        {
            Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Serial.printf(" - %u bytes written in %u ms\r\n", 2048 * 512, end);
    file.close();

    file = this->m_pFS->open(path, "r");
    start = millis();
    end = start;
    i = 0;
    if (file && !file.isDirectory())
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        Serial.print("- reading");
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F)
            {
                Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Serial.printf("- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    }
    else
    {
        Serial.println("- failed to open file for reading");
    }
}