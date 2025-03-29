#include "layouts/FluddIcon.h"
#include "al/string/StringTmp.h"
#include "al/util.hpp"
#include "al/util/MathUtil.h"
#include "logger.hpp"
#include "main.hpp"
#include "server/hns/HideAndSeekMode.hpp"
#include "math/seadVector.h"
#include "prim/seadSafeString.h"
#include "puppets/PuppetInfo.h"
#include "rs/util.hpp"
#include "server/Client.hpp"
#include <cstdio>
#include <cstring>
#include "server/Client.hpp"

FluddIcon::FluddIcon(const char* name, const al::LayoutInitInfo& initInfo) : al::LayoutActor(name)
{
    al::initLayoutActor(this, initInfo, "FluddIcon", 0);

    mInfo = GameModeManager::instance()->getInfo<HideAndSeekInfo>();

    initNerve(&nrvFluddIconEnd, 0);

    kill();

    //appear();
}

void FluddIcon::appear() {

    //al::startAction(this, "Appear", 0);

    al::setNerve(this, &nrvFluddIconWait);

    al::LayoutActor::appear();
}

bool FluddIcon::tryEnd() {
    if (!al::isNerve(this, &nrvFluddIconEnd)) {
        al::setNerve(this, &nrvFluddIconEnd);

        
        return true;
    }
    return false;
}

bool FluddIcon::tryStart() {
    if (!al::isNerve(this, &nrvHideAndSeekIconWait) &&
        !al::isNerve(this, &nrvHideAndSeekIconAppear)) {
        appear();

        return true;
    }

    return false;
}

void FluddIcon::exeAppear() {
    if (al::isActionEnd(this, 0)) {
        al::setNerve(this, &nrvHideAndSeekIconWait);
    }
}

void FluddIcon::exeWait() {
    if (al::isFirstStep(this)) {
        //al::startAction(this, "Wait", 0);
    }
}

void FluddIcon::exeEnd() {
    if (al::isFirstStep(this)) {
        //al::startAction(this, "End", 0);
    }

    //if (al::isActionEnd(this, 0)) {
        //kill();
    //}
}

namespace {
NERVE_IMPL(FluddIcon, Appear)
NERVE_IMPL(FluddIcon, Wait)
NERVE_IMPL(FluddIcon, End)
}  // namespace