#include <filemanager.h>

void CFileManager::Init()
{
    KeyValues *kv = new KeyValues("Config");

    kv->LoadFromFile(g_fileSystem, CONFIG_FILE);
    CFileManager::SetCacheTime(kv->GetInt("caching_time"));
    CFileManager::SetRconPassword(kv->GetString("rcon_password"));

    delete kv;
}

void CFileManager::CleanCache()
{
    KeyValues *kv = new KeyValues("Config");

    kv->LoadFromFile(g_fileSystem, CACHE_FILE);
    kv->Clear();
    kv->SaveToFile(g_fileSystem, CACHE_FILE);

    delete kv;
}

bool CFileManager::IsSteamIdCached(const char *steamid)
{
    KeyValues *kv = new KeyValues("Config");
    kv->LoadFromFile(g_fileSystem, CACHE_FILE);

    if (!kv->FindKey(steamid, false))
    {
        return false;
    }

    int time = kv->GetInt(steamid);
    bool value = time > std::time(0);

    delete kv;
    return value;
}

void CFileManager::AddSteamIdToCache(const char *steamid)
{
    KeyValues *kv = new KeyValues("Config");

    kv->LoadFromFile(g_fileSystem, CACHE_FILE);
    kv->SetInt(steamid, time(0) + (CFileManager::GetCacheTime() * 60));
    kv->SaveToFile(g_fileSystem, CACHE_FILE);

    delete kv;
}