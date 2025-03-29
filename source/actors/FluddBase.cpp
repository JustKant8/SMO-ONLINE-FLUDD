#include "actors/FluddBase.h"
#include "main.hpp"
#include "server/Client.hpp"
#include "al/LiveActor/LiveActor.h"


FluddBase::FluddBase(const char *name) : al::LiveActor(name) { 
    kill();
}

void FluddBase::init(al::ActorInitInfo const &info)
{
    al::initActorWithArchiveName(this, info, "FluddBase", nullptr);

    al::initNerve(this, &nrvFluddBaseWait, 0);

    mtxConnector = al::createMtxConnector(this);

    makeActorAlive();
    
}

void FluddBase::connect(LiveActor* m) {
    if (!al::isMtxConnectorConnecting(mtxConnector) || al::calcDistance(this, m) > 50.0f) { 
        al::setTrans(this, al::getTrans(m));
        al::attachMtxConnectorToJoint(mtxConnector, m, "Spine1");
    }
}

void FluddBase::activate() {

    al::tryStartActionIfNotPlaying(this, "Shoot");

    if (isSklAnimEnd(this, 0))
        al::setSklAnimFrame(this, 0, 0); 
}


void FluddBase::deactivate() {
    al::setSklAnimFrame(this, 0, 0);
}

void FluddBase::initAfterPlacement() {
    
}

void FluddBase::listenAppear() {
    this->appear();
}

bool FluddBase::receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                        al::HitSensor* target) {
    return false;
}

void FluddBase::attackSensor(al::HitSensor* source, al::HitSensor* target) {

    
}

// todo: no idea what 0x144 or 0x124 are

void FluddBase::control() {
    al::connectPoseQT(this, mtxConnector, *quat, *posOffset);
    isConnecting = al::isMtxConnectorConnecting(mtxConnector);


}

FluddBase* FluddBase::createFromFactory(al::ActorInitInfo const& rootInitInfo,
                                        al::PlacementInfo const& rootPlacementInfo,
                                        const char* propArchiveName) {
    al::ActorInitInfo actorInitInfo = al::ActorInitInfo();
    actorInitInfo.initViewIdSelf(&rootPlacementInfo, rootInitInfo);

    al::createActor createActor = actorInitInfo.mActorFactory->getCreator("FluddBase");

    if (!createActor) {
        return nullptr;
    }

    FluddBase* newActor = (FluddBase*)createActor("FluddBase");

    Logger::log("Creating Prop Actor: %s\n", propArchiveName);

    newActor->init(actorInitInfo);

    return reinterpret_cast<FluddBase*>(newActor);
}

void FluddBase::initAllActors(al::ActorInitInfo const& rootInfo,
                              al::PlacementInfo const& placement) {
    
    FluddBase::playerFludd = FluddBase::createFromFactory(rootInfo, placement, "FluddBase");

}

void FluddBase::updateCollider() {
    
    
}


void FluddBase::exeWait(void)  // 0x0
{
    if (al::isFirstStep(this)) {
        //al::startAction(this, "Wait");
    }
}

namespace
{
NERVE_IMPL(FluddBase, Wait)
} // namespace