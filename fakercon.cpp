#include <stdio.h>
#include "fakercon.h"
#include <string>
#include "filemanager.h"
#include "abstract.h"

void Debug(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[1024] = {};
	V_vsnprintf(buf, sizeof(buf) - 1, msg, args);

	META_CONPRINTF("%s\n", buf);

	va_end(args);
}

SH_DECL_HOOK1_void(ISource2GameClients, ClientFullyConnect, SH_NOATTRIB, false, CPlayerSlot);

FakeRcon g_FakeRcon;
IServerGameDLL *server = NULL;
ISource2GameClients *gameclients = NULL;
IVEngineServer2 *engine = NULL;
ICvar *icvar = NULL;
CFileManager *g_fileManager = NULL;
IFileSystem *g_fileSystem = NULL;

PlayerData g_playerData[MAXPLAYERS + 1];

const char *g_szRconPassword;

PLUGIN_EXPOSE(FakeRcon, g_FakeRcon);
bool FakeRcon::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, ISource2GameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_fileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);

	SH_ADD_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &FakeRcon::Hook_ClientFullyConnect, false);

	g_fileManager = new CFileManager();

	if (CommandLine()->HasParm("-fakercon"))
	{
		const char *rconPasswordConst = CommandLine()->ParmValue("-fakercon");
		g_szRconPassword = strdup(rconPasswordConst);

		Debug("[FAKE RCON] Fetching RCON from -fakercon command line");
	}
	else
	{
		g_szRconPassword = strdup(g_fileManager->GetRconPassword());
		Debug("[FAKE RCON] Fetching RCON from %s", CONFIG_FILE);
	}

	if (!g_szRconPassword || strlen(g_szRconPassword) < 4)
	{
		Debug("[FAKE RCON], please check the lengh of the password in game/csgo/%s or -fakercon command line", CONFIG_FILE);
	}
	else
	{
		Debug("[FAKE RCON] Fake rcon is %s", g_szRconPassword);
	}

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	return true;
}

bool FakeRcon::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &FakeRcon::Hook_ClientFullyConnect, false);

	delete g_fileManager;

	return true;
}

void FakeRcon::AllPluginsLoaded()
{
}

void FakeRcon::Hook_ClientFullyConnect(CPlayerSlot slot)
{
	PlayerData *pData = &g_playerData[slot.Get()];
	pData->logged = false; // Unlog for the security, the cache will be checked after the first fake_rcon cmd typed by the client
}

CON_COMMAND_EXTERN(fake_rcon_cache_clean, Command_FakeRconCacheClean, "Clean the fake rcon cache");
void Command_FakeRconCacheClean(const CCommandContext &context, const CCommand &args)
{
	CPlayerSlot slot = context.GetPlayerSlot();

	if (slot.Get() < 0)
	{
		return;
	}

	PlayerData *pData = &g_playerData[slot.Get()];

	if (!pData->logged)
	{
		const char *networkId = engine->GetPlayerNetworkIDString(slot.Get());
		if (!g_fileManager->IsSteamIdCached(networkId))
		{
			g_SMAPI->ClientConPrintf(slot, "First, enter the password with the fake_rcon_password command\n");
			return;
		}

		pData->logged = true;
	}

	g_fileManager->CleanCache();
	g_SMAPI->ClientConPrintf(slot, "Cache has been cleaned\n");
}

CON_COMMAND_EXTERN(fake_rcon, Command_FakeRcon, "fake_rcon {CMD}");
void Command_FakeRcon(const CCommandContext &context, const CCommand &args)
{
	CPlayerSlot slot = context.GetPlayerSlot();

	if (g_szRconPassword == nullptr || slot.Get() < 0)
	{
		return;
	}

	PlayerData *pData = &g_playerData[slot.Get()];

	if (!pData->logged)
	{
		const char *networkId = engine->GetPlayerNetworkIDString(slot.Get());
		if (!g_fileManager->IsSteamIdCached(networkId))
		{
			g_SMAPI->ClientConPrintf(slot, "First, enter the password with the fake_rcon_password command\n");
			return;
		}

		pData->logged = true;
	}

	char *commandString = const_cast<char *>(args.GetCommandString());
	const char *firstSpace = strchr(commandString, ' ');

	if (firstSpace == nullptr)
	{
		return;
	}

	const char *command = firstSpace + 1;
	engine->ServerCommand(command);
}

CON_COMMAND_EXTERN(fake_rcon_password, Command_FakeRconPassword, "fake_rcon_password {CMD}");
void Command_FakeRconPassword(const CCommandContext &context, const CCommand &args)
{
	CPlayerSlot slot = context.GetPlayerSlot();

	if (g_szRconPassword == nullptr || slot.Get() < 0)
	{
		return;
	}

	PlayerData *pData = &g_playerData[slot.Get()];

	if (pData->logged)
	{
		g_SMAPI->ClientConPrintf(slot, "You are already using the fake_rcon command\n");
		return;
	}

	const char *password = args[1];
	if (strcmp(password, g_szRconPassword) != 0)
	{
		g_SMAPI->ClientConPrintf(slot, "Bad password !\n");
		return;
	}

	g_SMAPI->ClientConPrintf(slot, "You can now use the fake_rcon command\n");
	pData->logged = true;

	const char *networkId = engine->GetPlayerNetworkIDString(slot.Get());
	g_fileManager->AddSteamIdToCache(networkId);
}

bool FakeRcon::Pause(char *error, size_t maxlen)
{
	return true;
}

bool FakeRcon::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *FakeRcon::GetLicense()
{
	return "Public Domain";
}

const char *FakeRcon::GetVersion()
{
	return "1.2.1";
}

const char *FakeRcon::GetDate()
{
	return __DATE__;
}

const char *FakeRcon::GetLogTag()
{
	return "STUB";
}

const char *FakeRcon::GetAuthor()
{
	return "Kriax";
}

const char *FakeRcon::GetDescription()
{
	return "Like the real RCON but it's not the real RCON, thank you Valve";
}

const char *FakeRcon::GetName()
{
	return "Fake RCON";
}

const char *FakeRcon::GetURL()
{
	return "https://kriax.ovh";
}
