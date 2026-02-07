#pragma once

#include <string>
#include "KeyValues.h"
#include <ctime>
#include "abstract.h"

#define CACHE_FILE "addons/configs/fake_rcon/cache.cfg"
#define CONFIG_FILE "addons/configs/fake_rcon/config.cfg"

extern IFileSystem *g_fileSystem;

class CFileManager
{
public:
    CFileManager()
    {
        Init();
    }

    ~CFileManager()
    {
        free(g_szRconPassword);
        g_szRconPassword = nullptr;
    }

    void CleanCache();
    bool IsSteamIdCached(const char *steamid);
    void AddSteamIdToCache(const char *steamid);

    int GetCacheTime() { return g_cacheTime; }
    const char *GetRconPassword() { return g_szRconPassword; }

private:
    int g_cacheTime = 120;
    char *g_szRconPassword = nullptr;
    void Init();
};

extern CFileManager *g_fileManager;
