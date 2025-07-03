#include "AccountMgr.h"
#include "Chat.h"
#include "Configuration/Config.h"
#include "ConfigValueCache.h"
#include "Creature.h"
#include "Define.h"
#include "GossipDef.h"
#include "Player.h"
#include "ScriptMgr.h"

enum class KeepOutConfig
{
    MAX_WARNINGS,
    ENABLED,
    TELEPORT_ENABLED,
    KICK_ENABLED,
    ANNOUNCER_ENABLED,

    NUM_CONFIGS,
};

class KeepOutConfigData : public ConfigValueCache<KeepOutConfig>
{
public:
    KeepOutConfigData() : ConfigValueCache(KeepOutConfig::NUM_CONFIGS) { };

    void BuildConfigCache() override
    {
        SetConfigValue<uint32>(KeepOutConfig::MAX_WARNINGS,     "MaxWarnings",              3);
        SetConfigValue<bool>(KeepOutConfig::ENABLED,            "KeepOutEnabled",           true);
        SetConfigValue<bool>(KeepOutConfig::TELEPORT_ENABLED,   "KeepOutTeleportEnabled",   true);
        SetConfigValue<bool>(KeepOutConfig::KICK_ENABLED,       "KeepOutKickPlayerEnabled", true);
        SetConfigValue<bool>(KeepOutConfig::ANNOUNCER_ENABLED,  "Announcer.Enable",         true);
    }
};

static KeepOutConfigData keepOutConfigData;

void teleportPlayer(Player* player)
{
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
    ChatHandler(player->GetSession()).PSendSysMessage("You have gone to a forbidden place your actions have been logged.");
}

void checkZoneKeepOut(Player* player)
{
    if (player->GetSession()->GetSecurity() >= SEC_MODERATOR)
        return;

    uint32 mapId = player->GetMapId();
    uint32 zoneId = player->GetZoneId();

    QueryResult result = WorldDatabase.Query("SELECT * FROM `mod_mko_map_lock` WHERE `mapId`={} AND `zoneID`={}", mapId, zoneId);

    if (!result)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    uint8 countWarnings = 1;

    QueryResult playerWarning = CharacterDatabase.Query("SELECT * FROM `mod_mko_map_exploit` WHERE `accountId`={}", accountId);

    if (!playerWarning)
    {
        CharacterDatabase.Execute("INSERT INTO `mod_mko_map_exploit` (`accountId`, `count`) VALUES ({}, {})", accountId, countWarnings);

        if (keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::TELEPORT_ENABLED))
            teleportPlayer(player);
    }
    else
    {
        countWarnings = (*playerWarning)[1].Get<uint8>() + 1;

        if (countWarnings <= keepOutConfigData.GetConfigValue<uint32>(KeepOutConfig::MAX_WARNINGS))
        {
            CharacterDatabase.Execute("UPDATE `mod_mko_map_exploit` SET `count`={} WHERE `accountId`={}", countWarnings, accountId);
            teleportPlayer(player);
        }
        else
        {
            if (keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::TELEPORT_ENABLED) && !keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::KICK_ENABLED))
                teleportPlayer(player);
            else if (keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::KICK_ENABLED))
                player->GetSession()->KickPlayer("MKO: Entering a place not allowed.", true);
            else
                ChatHandler(player->GetSession()).PSendSysMessage("You have gone to a forbidden place your actions have been logged.");
        }
    }
}

class KeepOutPlayerScript : public PlayerScript
{
public:
    KeepOutPlayerScript() : PlayerScript("KeepOutPlayerScript", {
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_UPDATE_ZONE
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::ANNOUNCER_ENABLED))
            ChatHandler(player->GetSession()).PSendSysMessage("This server is running the |cff4CFF00Keep Out |rmodule.");
    }

    void OnPlayerUpdateZone(Player* player, uint32 /*newZone*/,  uint32 /*newArea*/) override
    {
        if (keepOutConfigData.GetConfigValue<bool>(KeepOutConfig::ENABLED))
            checkZoneKeepOut(player);
    }
};

class KeepOutWorldScript : public WorldScript
{
public:
    KeepOutWorldScript() : WorldScript("KeepOutWorldScript", {
        WORLDHOOK_ON_BEFORE_CONFIG_LOAD
    }) { }

    void OnBeforeConfigLoad(bool reload) override
    {
        keepOutConfigData.Initialize(reload);
    }
};

void AddKeepOutScripts()
{
    new KeepOutWorldScript();
    new KeepOutPlayerScript();
}
