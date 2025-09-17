#pragma once

#include <tier0/System.h>
#include <string>
#include <vector>

enum SeekDir
{
    SEEK_DIR_SET,
    SEEK_DIR_CUR,
    SEEK_DIR_END  
};

struct SearchPath_t
{
    std::string basePath;
    std::string installDir;
};

abstract_class IFile
{
public:
    virtual void Close() = 0;
    virtual void Seek(size_t seek, SeekDir whence) = 0;
    virtual size_t Tell() = 0;
    virtual size_t Read(char* buf, size_t elemSize, size_t elemCount) = 0;
    virtual void Write(char* buf, size_t elemSize, size_t elemCount) = 0;
};

class CFileSystem : public ISystem
{
public:
    virtual bool Init();
    virtual void Shutdown();

    IFile* OpenFileRead(str_t name);
private:
    void AddSearchpath(std::string folder);
private:
    std::vector<SearchPath_t> m_Searchpaths;
};