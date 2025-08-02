#include <filemanager.h>
#include <tier0/platform.h>
#include "filesystem.h"
#include "tier0/dbg.h"

void CFileManager::Init()
{
    // Ensure the folder exists
    g_fileSystem->CreateDirHierarchy("addons/configs/fake_rcon", "MOD");

    // Create config.ini if missing
    if (!g_fileSystem->FileExists(CONFIG_FILE, "MOD"))
    {
        Debug("[FAKE RCON] Config file not found, creating default: %s", CONFIG_FILE);

        KeyValues *defaultKV = new KeyValues("Config");
        defaultKV->SetInt("caching_time", 120);
        defaultKV->SetString("rcon_password", "rconconfig");
        defaultKV->SaveToFile(g_fileSystem, CONFIG_FILE, "MOD");
        delete defaultKV;  // ✅ fixed
    }

    // Load the config
    KeyValues *kv = new KeyValues("Config");

    if (!kv->LoadFromFile(g_fileSystem, CONFIG_FILE, "MOD"))
    {
        Debug("[FAKE RCON] Failed to load config file: %s", CONFIG_FILE);
        g_szRconPassword = strdup("");
        delete kv;
        return;
    }

    KeyValues::AutoDelete autoDelete(kv);

    g_cacheTime = kv->GetInt("caching_time", 120);
    const char *szRconPassword = kv->GetString("rcon_password", nullptr);

    if (!szRconPassword)
    {
        Debug("[FAKE RCON] rcon_password missing in config");
        g_szRconPassword = strdup("");
        return;
    }

    char *copy = new char[strlen(szRconPassword) + 1];
    strcpy(copy, szRconPassword);
    g_szRconPassword = copy;

    // Create empty cache.ini if missing
    if (!g_fileSystem->FileExists(CACHE_FILE, "MOD"))
    {
        KeyValues *emptyCache = new KeyValues("Config");
        emptyCache->SaveToFile(g_fileSystem, CACHE_FILE, "MOD");
        delete emptyCache;  // ✅ fixed
        Debug("[FAKE RCON] Created empty cache file: %s", CACHE_FILE);
    }
}

void CFileManager::CleanCache()
{
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    kv->LoadFromFile(g_fileSystem, CACHE_FILE);
    kv->Clear();
    kv->SaveToFile(g_fileSystem, CACHE_FILE);
}

bool CFileManager::IsSteamIdCached(const char *steamid)
{
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    kv->LoadFromFile(g_fileSystem, CACHE_FILE);

    if (!kv->FindKey(steamid, false))
    {
        return false;
    }

    int time = kv->GetInt(steamid);
    bool value = time > std::time(0);

    return value;
}

void CFileManager::AddSteamIdToCache(const char *steamid)
{
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    kv->LoadFromFile(g_fileSystem, CACHE_FILE);
    kv->SetInt(steamid, time(0) + (CFileManager::GetCacheTime() * 60));
    kv->SaveToFile(g_fileSystem, CACHE_FILE);
}
