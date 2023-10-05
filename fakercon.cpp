#include <stdio.h>
#include "fakercon.h"
#include <string>
#include <iostream>
#include <fstream>
#include <fmtstr.h>

using namespace std;

SH_DECL_HOOK2_void(ISource2GameClients, ClientCommand, SH_NOATTRIB, 0, CPlayerSlot, const CCommand &);
SH_DECL_HOOK1_void(ISource2GameClients, ClientFullyConnect, SH_NOATTRIB, false, CPlayerSlot);

FakeRcon g_FakeRcon;
IServerGameDLL *server = NULL;
ISource2GameClients *gameclients = NULL;
IVEngineServer2 *engine = NULL;
ICvar *icvar = NULL;

PlayerData g_playerData[MAXPLAYERS + 1];

char *g_rconPassword;

PLUGIN_EXPOSE(FakeRcon, g_FakeRcon);
bool FakeRcon::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, ISource2GameClients, SOURCE2GAMECLIENTS_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	SH_ADD_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &FakeRcon::Hook_ClientFullyConnect, false);

	if (CommandLine()->HasParm("-fakercon"))
	{
		const char *rconPasswordConst = CommandLine()->ParmValue("-fakercon");
		g_rconPassword = strdup(rconPasswordConst);
		META_CONPRINTF("[FAKE RCON] Fetching RCON from -fakercon command line \n");
	}
	else
	{
		g_rconPassword = FakeRcon::getRconPasswordFromFile();
		META_CONPRINTF("[FAKE RCON] Fetching RCON from rcon.text \n");
	}

	if (g_rconPassword == nullptr)
	{
		META_CONPRINTF("[FAKE RCON], please check the lengh of the password in game/bin/<your_os>/rcon.txt or -fakercon command line \n");
	}
	else
	{
		META_CONPRINTF("[FAKE RCON] Fake rcon is %s\n", g_rconPassword);
	}

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL);

	return true;
}

bool FakeRcon::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &FakeRcon::Hook_ClientFullyConnect, false);

	return true;
}

void FakeRcon::AllPluginsLoaded()
{
}

void FakeRcon::Hook_ClientFullyConnect(CPlayerSlot slot)
{
	PlayerData *pData = &g_playerData[slot.Get()];
	pData->logged = false;
}

CON_COMMAND_EXTERN(fake_rcon, Command_FakeRcon, "fake_rcon {CMD}");
void Command_FakeRcon(const CCommandContext &context, const CCommand &args)
{
	CPlayerSlot slot = context.GetPlayerSlot();

	if (g_rconPassword == nullptr || slot.Get() < 0)
	{
		return;
	}

	PlayerData *pData = &g_playerData[slot.Get()];

	if (!pData->logged)
	{
		g_SMAPI->ClientConPrintf(slot, "First, enter the password with the fake_rcon_password command\n");
		return;
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

	if (g_rconPassword == nullptr || slot.Get() < 0)
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
	if (strcmp(password, g_rconPassword) != 0)
	{
		g_SMAPI->ClientConPrintf(slot, "Bad password !\n");
		return;
	}

	g_SMAPI->ClientConPrintf(slot, "You can now use the fake_rcon command\n");
	pData->logged = true;
}

char *FakeRcon::getRconPasswordFromFile()
{
	std::ifstream myfile("rcon.txt");

	if (!myfile.is_open())
	{
		return nullptr;
	}

	char buffer[256];
	myfile.getline(buffer, sizeof(buffer));
	myfile.close();

	if (strlen(buffer) < 4)
	{
		return nullptr;
	}

	char *result = new char[strlen(buffer) + 1];
	strcpy(result, buffer);

	return result;
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
	return "1.1.2";
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
	return "https://kriax.ovh/";
}
