#pragma once
#ifndef DS_SPINECHAR_H
#define DS_SPINECHAR_H

#include <ds/framework.h>
#include <spine/spine.h>

enum class SpineAnime {
    // continuous
    IDLE,
    DRAG,
    WORK,
    SLEEP,

    // one-shot
    STAND,
    TOUCH,
    WINK,
    VICTORY,
    DANCE,
    DIZZY,
};

enum class SpineState {
    IDLE,
    DRAG,
    WORK,
    SLEEP,
};

enum class SpineAction {
    DRAGUP,
    DRAGDOWN,
    BEGINWORK,
    ENDWORK,
    SLEEP,

    STAND,
    TOUCH,
    WINK,
    DANCE,
    DIZZY,
};

class Spine
{
private:
    std::map<SpineAnime, std::string> animeToName;

    // spine data
    spAtlas* atlas = NULL;
    spSkeletonBinary* skeletonBinary = NULL;
    spSkeletonData* skeletonData = NULL;
    spAnimationStateData* animationStateData = NULL;
    spSkeleton* skeleton = NULL;
    spAnimationState* animationState = NULL;

    SpineState state = SpineState::IDLE;

    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

public:
    Spine();

    BOOL CreateResources(PCSTR atlasPath, PCSTR skelPath);
    void DisposeResources();

    void SetAnimeName(SpineAnime anime, PCSTR name) { this->animeToName[anime] = name; }
    PCSTR GetAnimeName(SpineAnime anime) { return this->animeToName[anime].c_str(); }

    INT GetValidAnimeNamesCount() const { return this->skeletonData->animationsCount; }
    INT GetValidAnimeNames(PCSTR* names, INT length);

    BOOL SetAction(SpineAction action);

    BOOL Update(FLOAT elapseTime);
};

class SpineChar
{

private:
    HANDLE dataMutex        = NULL;

public:
    SpineChar(HWND targetWnd);

    BOOL LockSpine();
    BOOL UnLockSpine();

    BOOL StartRender();
    BOOL StopRender();
    BOOL SetSpine();

    BOOL Update(UINT milliseconds);

    BOOL CreateResources();
    BOOL FreeResources();
};


#endif // DS_SPINECHAR_H
