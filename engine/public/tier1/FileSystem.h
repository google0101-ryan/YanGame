#pragma once

#include <tier0/System.h>
#include <string>
#include <vector>

struct SearchPath_t
{
    std::string basePath;
    
    bool isZipFile;
};

class CFileSystem : public ISystem
{
public:
    virtual bool Init();
    virtual void Shutdown();
private:
    void AddSearchpath(std::string folder);
private:
    std::vector<SearchPath_t> m_Searchpaths;
};