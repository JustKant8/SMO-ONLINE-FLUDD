#include <stdint.h>
#include "container/seadSafeArray.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/PlayerModelHolder.h"
#include "game/StageScene/StageScene.h"
#include "gfx/seadColor.h"
#include "prim/seadSafeString.h"
#include "sead/math/seadVector.h"
#include "al/LiveActor/LiveActor.h"
#include "game/Player/PlayerFunction.h"
#include "game/Player/PlayerJointControlPartsDynamics.h"
#include "game/Player/PlayerConst.h"
#include "actors/FluddBase.h"
#include "actors/FluddTurbo.hpp"
#include "actors/FluddHover.hpp"
#include "actors/FluddRocket.hpp"

#include "layouts/FluddIcon.h"

class FLUDD{
private:
    //Fludd settings/values
    int fluddMode = 0;    // Modes: 0 = hover, 1 = rocket, 2 = turbo, 3 = normal
    float tank = 100.0f; //0 = empty, 100 = full
    float fluddRecharge = 1.0f;
    
    float fluddVel = 2.8f; //start off as hover velocity
    float chargeTimer = 0.0f; //charge-up bar
    float chargeTimerDecrease = 1.5f;
    float tankStopValue = 0.0f;
    float tankRunoutVal = 33.3f;
    bool recharging = false;
    int turboWaterDelay = 0;

    bool modelsInit = false;
    int doubleBoostFrames = 0;
    bool isFirstBoost = true;

    //Layout values
    sead::Vector2f tankWaterTrans = sead::Vector2f::zero;
    sead::Vector2f tankWaterScale = sead::Vector2f::zero;
    sead::Vector2f tankStopLevelPos = sead::Vector2f::zero;
    sead::Vector2f chargeBarTrans = sead::Vector2f::zero;
    sead::Vector2f chargeBarScale = sead::Vector2f::zero;

    
    void activateFludd();
    void deactivateFludd();
    float smoothVelocity(float from, float to, float g);//smooth velocity transition
    float stopTankValue();
    void updateLayout();
    void updateModels();

    //random
    bool tStopValueSet = false;
    const sead::SafeString action = "Fall"; // hover/rocket anim
    
    
public:
    //references
    StageScene* stageSceneRef;
    bool is2D;
    bool isHack;
    PlayerActorBase* playerBase;
    PlayerActorHakoniwa* mario;
    al::LiveActor* marioModel;
    FluddIcon* layout;

    bool lJCancel = false;

    //fludd parts
    FluddBase* base;
    ca::FluddTurbo* turbo;
    ca::FluddHover* hover;
    ca::FluddRocket* rocket;

    bool isPUnderWater = false;
    bool isPInWater = false;
    bool isPGrounded = false;
    bool stageChange = false;
    bool doOnce = false;
    bool setNrvGrounded = false;
    float fluddDischarge = 0.1f;

    bool stickActive = false; //left stick active for mode change


    //update
    void updateFludd(); //Main loop run every frame

    //Getters
    int getFluddMode();
    float getChargeTimer();

    //Is
    bool isTankEmpty(float f1); //Checks if tank is empty or not
    bool canFluddActivate(); //does most fludd checks

    //Setters
    void fluddTankFill();
    void changeFluddModeR(); //right d-pad press
    void changeFluddModeL(); //left d-pad press (or left stick press(toggleable))
    void setFluddModeValues();
    void resetLayout();
    void setRefs();
    void initModels(al::ActorInitInfo const& info);
    void firstTimeSetup();
    void setMarioPtr(PlayerActorHakoniwa* m);

    void setFluddMode(int i) { fluddMode = i; };
};

FLUDD& Fludd();