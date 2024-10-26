#include "game/StageScene/StageSceneStateServerConfig.hpp"
#include <cstdlib>
#include <cstring>
#include <math.h>
#include "al/string/StringTmp.h"
#include "al/util.hpp"
#include "basis/seadNew.h"
#include "game/SaveData/SaveDataAccessFunction.h"
#include "server/Client.hpp"
#include "container/seadPtrArray.h"
#include "container/seadSafeArray.h"
#include "logger.hpp"
#include "prim/seadSafeString.h"
#include "prim/seadStringUtil.h"
#include "rs/util/InputUtil.h"
#include "server/gamemode/GameModeBase.hpp"
#include "server/gamemode/GameModeFactory.hpp"
#include "server/gamemode/GameModeManager.hpp"

StageSceneStateServerConfig::StageSceneStateServerConfig(
    const char* name,
    al::Scene* scene,
    const al::LayoutInitInfo& initInfo,
    FooterParts* footerParts,
    GameDataHolder* dataHolder,
    bool
) : al::HostStateBase<al::Scene>(name, scene) {
    mFooterParts    = footerParts;
    mGameDataHolder = dataHolder;

    mMsgSystem = initInfo.getMessageSystem();

    mInput = new InputSeparator(mHost, true);

    // page 0 menu
    mMainOptions     = new SimpleLayoutMenu("ServerConfigMenu", "OptionSelect", initInfo, 0, false);
    mMainOptionsList = new CommonVerticalList(mMainOptions, initInfo, true);

    al::setPaneString(mMainOptions, "TxtOption", u"Server Configuration", 0);

    mMainOptionsList->unkInt1 = 1;

    mMainOptionsList->initDataNoResetSelected(mMainMenuOptionsCount);

    mMainMenuOptions = new sead::SafeArray<sead::WFixedSafeString<0x200>, mMainMenuOptionsCount>();
    mMainMenuOptions->mBuffer[ServerConfigOption::GAMEMODECONFIG].copy(u"Gamemode Config");
    mMainMenuOptions->mBuffer[ServerConfigOption::GAMEMODESWITCH].copy(u"Change Gamemode");
    mMainMenuOptions->mBuffer[ServerConfigOption::SETIP].copy(u"Change Server (needs restart)");
    mMainMenuOptions->mBuffer[ServerConfigOption::SETPORT].copy(u"Change Port (needs restart)");
    mMainMenuOptions->mBuffer[ServerConfigOption::HIDESERVER].copy(u"Hide Server in Debug (OFF)"); // TBD

    mMainOptionsList->addStringData(getMainMenuOptions(), "TxtContent");

    // gamemode select menu

    mModeSelect     = new SimpleLayoutMenu("GamemodeSelectMenu", "OptionSelect", initInfo, 0, false);
    mModeSelectList = new CommonVerticalList(mModeSelect, initInfo, true);

    al::setPaneString(mModeSelect, "TxtOption", u"Gamemode Selection", 0);

    const int modeCount = GameModeFactory::getModeCount();

    mModeSelectList->initDataNoResetSelected(modeCount);

    auto* modeSelectOptions = new sead::SafeArray<sead::WFixedSafeString<0x200>, modeCount>();

    for (size_t i = 0; i < modeCount; i++) {
        const char* modeName = GameModeFactory::getModeName(i);
        modeSelectOptions->mBuffer[i].convertFromMultiByteString(modeName, strlen(modeName));
    }

    mModeSelectList->addStringData(modeSelectOptions->mBuffer, "TxtContent");

    // gamemode config menu
    GameModeConfigMenuFactory factory("GameModeConfigFactory");
    for (int mode = 0; mode < factory.getMenuCount(); mode++) {
        GameModeEntry& entry = mGamemodeConfigMenus[mode];
        const char*    name  = factory.getMenuName(mode);

        entry.mMenu   = factory.getCreator(name)(name);
        entry.mLayout = new SimpleLayoutMenu("GameModeConfigMenu", "OptionSelect", initInfo, 0, false);
        entry.mList   = new CommonVerticalList(entry.mLayout, initInfo, true);

        al::setPaneString(entry.mLayout, "TxtOption", u"Gamemode Configuration", 0);
    }

    mCurrentList = mMainOptionsList;
    mCurrentMenu = mMainOptions;
}

void StageSceneStateServerConfig::init() {
    initNerve(&nrvStageSceneStateServerConfigMainMenu, 0);

    #ifdef EMU
    char ryujinx[0x10] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    nn::account::Uid user;
    nn::account::GetLastOpenedUser(&user);
    if (memcmp(user.data, ryujinx, 0x10) == 0) {
        Client::showUIMessage(u"Error: Ryujinx default profile detected.\nYou have to create a new user profile!");
    }
    #endif
}

void StageSceneStateServerConfig::appear(void) {
    mCurrentMenu->startAppear("Appear");
    al::NerveStateBase::appear();
}

void StageSceneStateServerConfig::kill(void) {
    mCurrentMenu->startEnd("End");
    al::NerveStateBase::kill();

    if (Client::hasServerChanged()) {
        #if EMU
        Client::showUIMessage(u"You changed the server and have to restart the emulator now.");
        #else
        Client::showUIMessage(u"You changed the server and have to restart the game now.");
        #endif
    }
}

al::MessageSystem* StageSceneStateServerConfig::getMessageSystem(void) const {
    return mMsgSystem;
}

void StageSceneStateServerConfig::exeMainMenu() {
    if (al::isFirstStep(this)) {
        activateInput();
    }

    mInput->update();

    mCurrentList->update();

    if (mInput->isTriggerUiUp()) {
        mCurrentList->up();
    }

    if (mInput->isTriggerUiDown()) {
        mCurrentList->down();
    }

    if (rs::isTriggerUiCancel(mHost)) {
        kill();
    }

    if (rs::isTriggerUiDecide(mHost)) {
        deactivateInput();
    }

    if (mIsDecideConfig && mCurrentList->isDecideEnd()) {
        switch (mCurrentList->mCurSelected) {
            case ServerConfigOption::GAMEMODECONFIG: {
                al::setNerve(this, &nrvStageSceneStateServerConfigGamemodeConfig);
                break;
            }
            case ServerConfigOption::GAMEMODESWITCH: {
                al::setNerve(this, &nrvStageSceneStateServerConfigGamemodeSelect);
                break;
            }
            case ServerConfigOption::SETIP: {
                al::setNerve(this, &nrvStageSceneStateServerConfigOpenKeyboardIP);
                break;
            }
            case ServerConfigOption::SETPORT: {
                al::setNerve(this, &nrvStageSceneStateServerConfigOpenKeyboardPort);
                break;
            }
            case ServerConfigOption::HIDESERVER: {
                al::setNerve(this, &nrvStageSceneStateServerConfigHideServer);
                break;
            }
            default: {
                kill();
                break;
            }
        }
    }
}

void StageSceneStateServerConfig::exeOpenKeyboardIP() {
    if (al::isFirstStep(this)) {
        mCurrentList->deactivate();

        Client::getKeyboard()->setHeaderText(u"Set a server address below.");
        Client::getKeyboard()->setSubText(u"");

        bool isSave = Client::openKeyboardIP(); // anything that happens after this will be ran after the keyboard closes

        al::startHitReaction(mCurrentMenu, "リセット", 0);

        if (isSave) {
            al::setNerve(this, &nrvStageSceneStateServerConfigSaveData);
        } else {
            al::setNerve(this, &nrvStageSceneStateServerConfigMainMenu);
        }
    }
}

void StageSceneStateServerConfig::exeOpenKeyboardPort() {
    if (al::isFirstStep(this)) {
        mCurrentList->deactivate();

        Client::getKeyboard()->setHeaderText(u"Set a server port below.");
        Client::getKeyboard()->setSubText(u"");

        bool isSave = Client::openKeyboardPort(); // anything that happens after this will be ran after the keyboard closes

        al::startHitReaction(mCurrentMenu, "リセット", 0);

        if (isSave) {
            al::setNerve(this, &nrvStageSceneStateServerConfigSaveData);
        } else {
            al::setNerve(this, &nrvStageSceneStateServerConfigMainMenu);
        }
    }
}

void StageSceneStateServerConfig::exeHideServer() {
    if (al::isFirstStep(this)) {
        Client::toggleServerHidden();
        mMainOptionsList->initDataNoResetSelected(mMainMenuOptionsCount);
        mMainOptionsList->addStringData(getMainMenuOptions(), "TxtContent");
        mMainOptionsList->updateParts();
        al::setNerve(this, &nrvStageSceneStateServerConfigSaveData);
    }
}

void StageSceneStateServerConfig::exeGamemodeConfig() {
    if (al::isFirstStep(this)) {
        mGamemodeConfigMenu = &mGamemodeConfigMenus[GameModeManager::instance()->getGameMode()];

        mGamemodeConfigMenu->mList->initDataNoResetSelected(mGamemodeConfigMenu->mMenu->getMenuSize());
        mGamemodeConfigMenu->mList->addStringData(mGamemodeConfigMenu->mMenu->getStringData(), "TxtContent");

        mCurrentList = mGamemodeConfigMenu->mList;
        mCurrentMenu = mGamemodeConfigMenu->mLayout;

        subMenuStart();
    }

    subMenuUpdate();

    if (mIsDecideConfig && mCurrentList->isDecideEnd()) {
        GameModeConfigMenu::UpdateAction action = mGamemodeConfigMenu->mMenu->updateMenu(mCurrentList->mCurSelected);
        switch (action) {
            case GameModeConfigMenu::UpdateAction::CLOSE: {
                endSubMenu();
                break;
            }
            case GameModeConfigMenu::UpdateAction::REFRESH: {
                subMenuRefresh();
                break;
            }
            case GameModeConfigMenu::UpdateAction::NOOP: {
                activateInput();
                break;
            }
        }
    }
}

void StageSceneStateServerConfig::exeGamemodeSelect() {
    if (al::isFirstStep(this)) {
        mCurrentList = mModeSelectList;
        mCurrentMenu = mModeSelect;

        subMenuStart();
    }

    subMenuUpdate();

    if (mIsDecideConfig && mCurrentList->isDecideEnd()) {
        Logger::log("Setting Server Mode to: %d\n", mCurrentList->mCurSelected);
        GameModeManager::instance()->setMode(static_cast<GameMode>(mCurrentList->mCurSelected));
        endSubMenu();
    }
}

void StageSceneStateServerConfig::exeSaveData() {
    if (al::isFirstStep(this)) {
        SaveDataAccessFunction::startSaveDataWrite(mGameDataHolder);
    }

    if (SaveDataAccessFunction::updateSaveDataAccess(mGameDataHolder, false)) {
        al::startHitReaction(mCurrentMenu, "リセット", 0);
        al::setNerve(this, &nrvStageSceneStateServerConfigMainMenu);
    }
}

void StageSceneStateServerConfig::endSubMenu() {
    mCurrentList->deactivate();
    mCurrentMenu->kill();

    mCurrentList = mMainOptionsList;
    mCurrentMenu = mMainOptions;

    mCurrentMenu->startAppear("Appear");

    al::startHitReaction(mCurrentMenu, "リセット", 0);
    al::setNerve(this, &nrvStageSceneStateServerConfigMainMenu);
}

void StageSceneStateServerConfig::subMenuStart() {
    mCurrentList->deactivate();

    mCurrentMenu->kill();

    activateInput();

    mCurrentMenu->startAppear("Appear");
}

void StageSceneStateServerConfig::subMenuUpdate() {
    mInput->update();

    mCurrentList->update();

    if (mInput->isTriggerUiUp()) {
        mCurrentList->up();
    }

    if (mInput->isTriggerUiDown()) {
        mCurrentList->down();
    }

    if (rs::isTriggerUiCancel(mHost)) {
        endSubMenu();
    }

    if (rs::isTriggerUiDecide(mHost)) {
        deactivateInput();
    }
}

void StageSceneStateServerConfig::subMenuRefresh() {
    mGamemodeConfigMenu = &mGamemodeConfigMenus[GameModeManager::instance()->getGameMode()];
    mGamemodeConfigMenu->mList->initDataNoResetSelected(mGamemodeConfigMenu->mMenu->getMenuSize());
    mGamemodeConfigMenu->mList->addStringData(mGamemodeConfigMenu->mMenu->getStringData(), "TxtContent");
    mGamemodeConfigMenu->mList->updateParts();
    activateInput();
}

const sead::WFixedSafeString<0x200>* StageSceneStateServerConfig::getMainMenuOptions() {
    mMainMenuOptions->mBuffer[ServerConfigOption::HIDESERVER].copy(
        Client::isServerHidden()
        ? u"Hide Server in Debug (ON) "
        : u"Hide Server in Debug (OFF)"
    );

    return mMainMenuOptions->mBuffer;
}

void StageSceneStateServerConfig::activateInput() {
    mInput->reset();
    mCurrentList->activate();
    mCurrentList->appearCursor();
    mIsDecideConfig = false;
}

void StageSceneStateServerConfig::deactivateInput() {
    al::startHitReaction(mCurrentMenu, "決定", 0);
    mCurrentList->endCursor();
    mCurrentList->decide();
    mIsDecideConfig = true;
}

namespace {
    NERVE_IMPL(StageSceneStateServerConfig, MainMenu)
    NERVE_IMPL(StageSceneStateServerConfig, OpenKeyboardIP)
    NERVE_IMPL(StageSceneStateServerConfig, OpenKeyboardPort)
    NERVE_IMPL(StageSceneStateServerConfig, HideServer)
    NERVE_IMPL(StageSceneStateServerConfig, GamemodeConfig)
    NERVE_IMPL(StageSceneStateServerConfig, GamemodeSelect)
    NERVE_IMPL(StageSceneStateServerConfig, SaveData)
}
