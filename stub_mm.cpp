#include <stdio.h>
#include "stub_mm.h"
#include "iserver.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

SH_DECL_HOOK2_void(ISource2GameClients, ClientCommand, SH_NOATTRIB, 0, CPlayerSlot, const CCommand &);
SH_DECL_HOOK1_void(ISource2GameClients, ClientFullyConnect, SH_NOATTRIB, false, CPlayerSlot);

StubPlugin g_StubPlugin;
IServerGameDLL *server = NULL;
ISource2GameClients *gameclients = NULL;
IVEngineServer2 *engine = NULL;
ICvar *icvar = NULL;

PlayerData g_playerData[MAXPLAYERS + 1];

char *g_rconPassword;

PLUGIN_EXPOSE(StubPlugin, g_StubPlugin);
bool StubPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, ISource2GameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	SH_ADD_HOOK_MEMFUNC(ISource2GameClients, ClientCommand, gameclients, this, &StubPlugin::Hook_ClientCommand, false);
	SH_ADD_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &StubPlugin::Hook_ClientFullyConnect, false);

	g_pCVar = icvar;

	g_rconPassword = StubPlugin::getRconPassword();

	if (g_rconPassword == nullptr)
	{
		META_CONPRINTF("- FAKE RCON DISABLE, please check the lengh of the password in game/bin/win64/rcon.txt \n");
	}
	else
	{
		META_CONPRINTF("- FAKE RCON ENABLE, fake rcon is %s\n", g_rconPassword);
	}

	return true;
}

bool StubPlugin::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(ISource2GameClients, ClientCommand, gameclients, this, &StubPlugin::Hook_ClientCommand, false);
	SH_REMOVE_HOOK_MEMFUNC(ISource2GameClients, ClientFullyConnect, gameclients, this, &StubPlugin::Hook_ClientFullyConnect, false);

	return true;
}

void StubPlugin::AllPluginsLoaded()
{
}

void StubPlugin::Hook_ClientFullyConnect(CPlayerSlot slot)
{
	PlayerData *pData = &g_playerData[slot.Get()];
	pData->logged = false;
}

void StubPlugin::Hook_ClientCommand(CPlayerSlot slot, const CCommand &args)
{
	if (g_rconPassword == nullptr)
	{
		return;
	}

	const char *command = args[0];
	PlayerData *pData = &g_playerData[slot.Get()];

	if (strcmp(command, "fake_rcon_password") == 0)
	{
		if (pData->logged)
		{
			return;
		}

		const char *password = args[1];

		if (strcmp(password, g_rconPassword) != 0)
		{
			return;
		}

		pData->logged = true;
	}

	if (strcmp(command, "fake_rcon") == 0)
	{
		char *result = StubPlugin::getCommand(const_cast<char *>(args.GetCommandString()));

		if (!pData->logged)
		{
			return;
		}

		engine->ServerCommand(result);
	}
}

char *StubPlugin::getRconPassword()
{
	std::ifstream myfile("rcon.txt");

	if (!myfile.is_open())
	{
		return nullptr;
	}

	char buffer[256];
	myfile.getline(buffer, sizeof(buffer));
	myfile.close();

	if (std::strlen(buffer) < 4)
	{
		return nullptr;
	}

	char *result = new char[std::strlen(buffer) + 1];
	std::strcpy(result, buffer);

	return result;
}

char *StubPlugin::getCommand(char *input)
{
	char *token = strtok(input, " ");

	if (token != NULL)
	{
		char *result = strtok(NULL, "");
		return result;
	}

	return nullptr;
}

bool StubPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool StubPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *StubPlugin::GetLicense()
{
	return "Public Domain";
}

const char *StubPlugin::GetVersion()
{
	return "1.0.0.0";
}

const char *StubPlugin::GetDate()
{
	return __DATE__;
}

const char *StubPlugin::GetLogTag()
{
	return "STUB";
}

const char *StubPlugin::GetAuthor()
{
	return "Kriax";
}

const char *StubPlugin::GetDescription()
{
	return "Like the real RCON but it's not the real RCON, thank you Valve";
}

const char *StubPlugin::GetName()
{
	return "Fake RCON";
}

const char *StubPlugin::GetURL()
{
	return "https://kriax.ovh/";
}