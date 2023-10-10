#include <filemanager.h>

void CFileManager::Init()
{
    KeyValues *kv = new KeyValues("Config");

    kv->LoadFromFile(g_fileSystem, CONFIG_FILE);
    KeyValues::AutoDelete autoDelete(kv);

    g_cacheTime = kv->GetInt("caching_time", 120);
    const char *szRconPassword = kv->GetString("rcon_password", nullptr);

    char *copy = new char[strlen(szRconPassword) + 1]; 
    strcpy(copy, szRconPassword);
    g_szRconPassword = copy;
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