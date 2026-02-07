#include <filemanager.h>
#include <tier0/platform.h>
#include "filesystem.h"
#include "tier0/dbg.h"

static bool MigrateFile(const char *oldPath, const char *newPath)
{
    if (!g_fileSystem->FileExists(oldPath, "MOD") || g_fileSystem->FileExists(newPath, "MOD"))
        return false;

    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    if (!kv->LoadFromFile(g_fileSystem, oldPath, "MOD"))
    {
        Debug("[FAKE RCON] Migration: failed to load %s, skipping", oldPath);
        return false;
    }

    if (!kv->SaveToFile(g_fileSystem, newPath, "MOD"))
    {
        Debug("[FAKE RCON] Migration: failed to write %s, keeping original", oldPath);
        return false;
    }

    // New file written successfully, safe to remove old one
    g_fileSystem->RemoveFile(oldPath, "MOD");
    Debug("[FAKE RCON] Migrated %s -> %s", oldPath, newPath);
    return true;
}

void CFileManager::Init()
{
    // Ensure the folder exists
    g_fileSystem->CreateDirHierarchy("addons/configs/fake_rcon", "MOD");

    // Migrate old .ini files to .cfg
    MigrateFile("addons/configs/fake_rcon/config.ini", CONFIG_FILE);
    MigrateFile("addons/configs/fake_rcon/cache.ini", CACHE_FILE);

    // Create config.cfg if missing
    if (!g_fileSystem->FileExists(CONFIG_FILE, "MOD"))
    {
        Debug("[FAKE RCON] Config file not found, creating default: %s", CONFIG_FILE);

        KeyValues *defaultKV = new KeyValues("Config");
        defaultKV->SetInt("caching_time", 120);
        defaultKV->SetString("rcon_password", "changeme");
        defaultKV->SaveToFile(g_fileSystem, CONFIG_FILE, "MOD");
        delete defaultKV;  // ✅ fixed
        delete defaultKV;
    }

    // Load the config
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    if (!kv->LoadFromFile(g_fileSystem, CONFIG_FILE, "MOD"))
    {
        Debug("[FAKE RCON] Failed to load config file: %s", CONFIG_FILE);
        Debug("[FAKE RCON] The config must be in Valve KeyValues format, e.g.:");
        Debug("[FAKE RCON]   \"Config\" { \"rcon_password\" \"mypassword\" \"caching_time\" \"120\" }");
        g_szRconPassword = nullptr;
        return;
    }

    g_cacheTime = kv->GetInt("caching_time", 120);
    const char *szRconPassword = kv->GetString("rcon_password", nullptr);

    if (!szRconPassword)
    {
        Debug("[FAKE RCON] rcon_password missing in config");
        g_szRconPassword = nullptr;
        return;
    }

    g_szRconPassword = strdup(szRconPassword);

    // Create empty cache.cfg if missing
    if (!g_fileSystem->FileExists(CACHE_FILE, "MOD"))
    {
        KeyValues *emptyCache = new KeyValues("Config");
        emptyCache->SaveToFile(g_fileSystem, CACHE_FILE, "MOD");
        delete emptyCache;
        Debug("[FAKE RCON] Created empty cache file: %s", CACHE_FILE);
    }
}

void CFileManager::CleanCache()
{
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    kv->LoadFromFile(g_fileSystem, CACHE_FILE, "MOD");
    kv->Clear();
    kv->SaveToFile(g_fileSystem, CACHE_FILE, "MOD");
}

bool CFileManager::IsSteamIdCached(const char *steamid)
{
    KeyValues *kv = new KeyValues("Config");
    KeyValues::AutoDelete autoDelete(kv);

    kv->LoadFromFile(g_fileSystem, CACHE_FILE, "MOD");

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

    kv->LoadFromFile(g_fileSystem, CACHE_FILE, "MOD");
    kv->SetInt(steamid, time(0) + (CFileManager::GetCacheTime() * 60));
    kv->SaveToFile(g_fileSystem, CACHE_FILE, "MOD");
}
