#include <tier1/FileSystem.h>
#include <tier1/Cvar.h>

#include <tier0/log.h>

#include <filesystem>

class CStdioFile : public IFile
{
public:
    CStdioFile(str_t fName, bool read)
    {
        m_pFd = fopen(fName, read ? "rb" : "wb");
    }

    virtual void Close() { fclose(m_pFd); }
    virtual void Seek(size_t seek, SeekDir whence) { fseek(m_pFd, seek, whence); }
    virtual size_t Tell() { return ftell(m_pFd); }
    virtual size_t Read(char* buf, size_t elemSize, size_t elemCount) { return fread(buf, elemSize, elemCount, m_pFd); }
    virtual void Write(char* buf, size_t elemSize, size_t elemCount) { fwrite(buf, elemSize, elemCount, m_pFd); }
private:
    FILE* m_pFd;
};

CCvar fs_basePath("fs_basepath", "Assets", "Sets the root folder to be appended to the executable folder for accessing assets", CF_ARCHIVE);

bool CFileSystem::Init()
{
    auto exeDir = std::filesystem::current_path();

    m_Searchpaths.push_back({fs_basePath.GetValue(), exeDir});

    if (!std::filesystem::exists(exeDir / fs_basePath.GetValue()))
    {
        LOG_FATAL("Failed to find path to game assets (Is your game installed correctly?)");
        return false;
    }

    return true;
}

void CFileSystem::Shutdown()
{
    m_Searchpaths.clear();
}

IFile *CFileSystem::OpenFileRead(str_t name)
{
    IFile* ret = nullptr;

    for (int i = 0; i < m_Searchpaths.size(); i++)
    {
        auto& sp = m_Searchpaths[i];

        auto fullPath = std::filesystem::path(sp.installDir) / sp.basePath / name;

        if (std::filesystem::exists(fullPath))
        {
            LOG_INFO("File '{}' found at '{}'\n", name, fullPath.string());
            ret = new CStdioFile(fullPath.c_str(), true);
            return ret;
        }
    }

    LOG_ERROR("File \"{}\" not found\n", name);
    return nullptr;
}