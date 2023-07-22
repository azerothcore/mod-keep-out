#include "Configuration/Config.h"
#include "Player.h"
#include "Creature.h"
#include "AccountMgr.h"
#include "ScriptMgr.h"
#include "Define.h"
#include "GossipDef.h"
#include "Chat.h"

struct MKO
{
    uint32 maxWarnings;
    bool keepOutEnabled;
};

MKO mko;

void checkMapsAndZone(Player* player)
{
    if (player->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
        return;

    QueryResult result = WorldDatabase.Query("SELECT `mapId` FROM `mko_map_lock` WHERE `mapId`={}", player->GetMapId());

    if (!result)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    uint32 mapId = player->GetMap()->GetId();
    uint32 areaId = player->GetAreaId();
    uint8 countWarnings = 1;

    QueryResult playerWarning = CharacterDatabase.Query("SELECT * FROM `mko_map_exploit` WHERE `accountId`={}", accountId);

    if (!playerWarning)
    {
        CharacterDatabase.Execute("INSERT INTO `mko_map_exploit` (`accountId`, `map`, `area`, `count`) VALUES ({}, {}, {}, {})", accountId, mapId, areaId, countWarnings);
    }
    else
    {
        countWarnings = (*result)[3].Get<uint8>() + 1;

        if (countWarnings <= mko.maxWarnings)
        {
            CharacterDatabase.Execute("UPDATE `mko_map_exploit` SET `count`={} WHERE `accountId`={}", countWarnings, accountId);

            if (player->GetTeamId() == TEAM_HORDE)
            {
                /* Orgrimmar */
                player->TeleportTo(1, 1629.85f, -4373.64f, 31.5573f, 3.69762f);
            }
            else
            {
                /* Stormwind */
                player->TeleportTo(0, -8833.38f, 628.628f, 94.0066f, 1.06535f);
            }
        }
        else
        {
            player->GetSession()->KickPlayer("MKO: Entering a place not allowed.", true);
        }
    }
}

class KeepOut : public PlayerScript
{
public:
    KeepOut() : PlayerScript("KeepOut") { }

    void OnLogin(Player* player)
    {
        if (sConfigMgr->GetOption<bool>("Announcer.Enable", true))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("This server is running the |cff4CFF00Keepout |rmodule.");
        }
    }

    void OnMapChanged(Player* player)
    {
        if (mko.keepOutEnabled)
        {
            checkMapsAndZone(player);
        }
    }

    void OnUpdateZone(Player* player, uint32 /*newZone */,  uint32 /*newArea*/)
    {
        if (mko.keepOutEnabled)
        {
            checkMapsAndZone(player);
        }
    }
};

class KeepoutConf : public WorldScript
{
public:
    KeepoutConf() : WorldScript("KeepoutConf") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload)
        {
            mko.maxWarnings = sConfigMgr->GetOption<int>("MaxWarnings", 3);
            mko.keepOutEnabled = sConfigMgr->GetOption<bool>("KeepOutEnabled", true);
        }
    }
};

void AddKeepOutScripts()
{
    new KeepoutConf();
    new KeepOut();
}
