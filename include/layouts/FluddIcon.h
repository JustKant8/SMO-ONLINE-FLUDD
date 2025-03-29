#pragma once

#include "al/layout/LayoutActor.h"
#include "al/layout/LayoutInitInfo.h"
#include "al/util/NerveUtil.h"

#include "container/seadPtrArray.h"
#include "logger.hpp"
#include "math/seadVector.h"

// TODO: kill layout if going through loading zone or paused

class FluddIcon : public al::LayoutActor {
public:
    FluddIcon(const char* name, const al::LayoutInitInfo& initInfo);

    void appear() override;

    bool tryStart();
    bool tryEnd();

    void exeAppear();
    void exeWait();
    void exeEnd();

    private:
    struct HideAndSeekInfo* mInfo;

};

namespace {
NERVE_HEADER(FluddIcon, Appear)
NERVE_HEADER(FluddIcon, Wait)
NERVE_HEADER(FluddIcon, End)
}