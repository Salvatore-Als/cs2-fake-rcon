#define MAXPLAYERS 64

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>

class FakeRcon : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen);
	bool Unpause(char *error, size_t maxlen);
	void AllPluginsLoaded();

	void Hook_ClientFullyConnect( CPlayerSlot nSlot );

public:
	const char *GetAuthor();
	const char *GetName();
	const char *GetDescription();
	const char *GetURL();
	const char *GetLicense();
	const char *GetVersion();
	const char *GetDate();
	const char *GetLogTag();
};

struct PlayerData
{
	bool logged;
};

extern FakeRcon g_FakeRcon;

PLUGIN_GLOBALVARS();