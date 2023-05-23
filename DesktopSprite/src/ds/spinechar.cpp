#include <ds/framework.h>

#include <ds/utils.h>
#include <spine/spine.h>

#include <ds/spinechar.h>

Spine::Spine()
{
    // continuous
    this->animeToName[SpineAnime::IDLE] = "normal";
    this->animeToName[SpineAnime::DRAG] = "tuozhuai";
    this->animeToName[SpineAnime::WORK] = "walk";
    this->animeToName[SpineAnime::SLEEP] = "sleep";

    // one-shot
    this->animeToName[SpineAnime::STAND] = "stand";
    this->animeToName[SpineAnime::TOUCH] = "touch";
    this->animeToName[SpineAnime::WINK] = "motou";
    this->animeToName[SpineAnime::VICTORY] = "victory";
    this->animeToName[SpineAnime::DANCE] = "dance";
    this->animeToName[SpineAnime::DIZZY] = "yun";
}

BOOL Spine::CreateResources(PCSTR atlasPath, PCSTR skelPath)
{
    if (!(this->atlas =              spAtlas_createFromFile(atlasPath, 0)))                                  return FALSE;
    if (!(this->skeletonBinary =     spSkeletonBinary_create(this->atlas)))                                  return FALSE;
    if (!(this->skeletonData =       spSkeletonBinary_readSkeletonDataFile(this->skeletonBinary, skelPath))) return FALSE;
    if (!(this->animationStateData = spAnimationStateData_create(this->skeletonData)))                       return FALSE;
    if (!(this->skeleton =           spSkeleton_create(this->skeletonData)))                                 return FALSE;
    if (!(this->animationState =     spAnimationState_create(animationStateData)))                           return FALSE;

    animationStateData->defaultMix = 0.25f;

    return TRUE;
}

void Spine::DisposeResources()
{
    if (this->animationState)     spAnimationState_dispose(this->animationState);
    if (this->skeleton)           spSkeleton_dispose(this->skeleton);
    if (this->animationStateData) spAnimationStateData_dispose(this->animationStateData);
    if (this->skeletonData)       spSkeletonData_dispose(this->skeletonData);
    if (this->skeletonBinary)     spSkeletonBinary_dispose(this->skeletonBinary);
    if (this->atlas)              spAtlas_dispose(this->atlas);
}

INT Spine::GetValidAnimeNames(PCSTR* names, INT length)
{
    if (length < this->skeletonData->animationsCount)
    {
        return 0;
    }

    for (int i = 0; i < this->skeletonData->animationsCount; i++)
    {
        names[i] = this->skeletonData->animations[i]->name;
    }

    return this->skeletonData->animationsCount;
}

BOOL Spine::SetAnime(SpineAnime anime, BOOL isOneShot, SpineAnime rollin)
{
    spAnimationState_setAnimationByName(this->animationState, 0, this->animeToName[anime].c_str(), 1);
    if (isOneShot)
    {
        spAnimationState_addAnimationByName(this->animationState, 0, this->animeToName[rollin].c_str(), 1, 0);
    }
    return TRUE;
}

BOOL Spine::SetAction(SpineAction action)
{
    //case SpineAction::DRAGUP:
    //case SpineAction::DRAGDOWN:
    //case SpineAction::BEGINWORK:
    //case SpineAction::ENDWORK:
    //case SpineAction::SLEEP:

    //case SpineAction::STAND:
    //case SpineAction::TOUCH:
    //case SpineAction::WINK:
    //case SpineAction::DANCE:
    //case SpineAction::DIZZY:
    switch (this->state)
    {
    case SpineState::IDLE:
        switch (action)
        {
        case SpineAction::DRAGUP:
            this->state = SpineState::DRAG;
            this->SetAnime(SpineAnime::DRAG);
            break;
        case SpineAction::BEGINWORK:
            this->state = SpineState::WORK;
            this->SetAnime(SpineAnime::WORK);
            break;
        case SpineAction::SLEEP:
            this->state = SpineState::SLEEP;
            this->SetAnime(SpineAnime::SLEEP);
            break;
        case SpineAction::STAND:
            this->SetAnime(SpineAnime::STAND, TRUE);
            break;
        case SpineAction::TOUCH:
            this->SetAnime(SpineAnime::TOUCH, TRUE);
            break;
        case SpineAction::WINK:
            this->SetAnime(SpineAnime::WINK, TRUE);
            break;
        case SpineAction::DANCE:
            this->SetAnime(SpineAnime::DANCE, TRUE);
            break;
        case SpineAction::DIZZY:
            this->SetAnime(SpineAnime::DIZZY, TRUE);
            break;
        default:
            this->SetAnime(SpineAnime::IDLE);
            break;
        }
    case SpineState::DRAG:
        switch (action)
        {
        case SpineAction::DRAGDOWN:
            this->state = SpineState::IDLE;
            this->SetAnime(SpineAnime::IDLE);
            break;
        default:
            this->SetAnime(SpineAnime::DRAG); 
            break;
        }
    case SpineState::WORK:
        switch (action)
        {
        case SpineAction::ENDWORK:
            this->state = SpineState::IDLE;
            this->SetAnime(SpineAnime::IDLE);
            break;
        case SpineAction::TOUCH:
            this->SetAnime(SpineAnime::TOUCH, TRUE, SpineAnime::WORK);
            break;
        case SpineAction::WINK:
            this->SetAnime(SpineAnime::WINK, TRUE, SpineAnime::WORK);
            break;
        case SpineAction::DIZZY:
            this->SetAnime(SpineAnime::DIZZY, TRUE, SpineAnime::WORK);
            break;
        default:
            this->SetAnime(SpineAnime::WORK);
            break;
        }
    case SpineState::SLEEP:
        switch (action)
        {
        case SpineAction::DRAGUP:
            this->state = SpineState::DRAG;
            this->SetAnime(SpineAnime::DRAG);
            break;
        case SpineAction::BEGINWORK:
            this->state = SpineState::WORK;
            this->SetAnime(SpineAnime::WORK);
            break;
        case SpineAction::STAND:
            this->SetAnime(SpineAnime::STAND, TRUE);
            break;
        case SpineAction::TOUCH:
            this->SetAnime(SpineAnime::TOUCH, TRUE);
            break;
        case SpineAction::WINK:
            this->SetAnime(SpineAnime::WINK, TRUE);
            break;
        case SpineAction::DANCE:
            this->SetAnime(SpineAnime::STAND, TRUE);
            break;
        case SpineAction::DIZZY:
            this->SetAnime(SpineAnime::DIZZY, TRUE);
            break;
        default:
            this->SetAnime(SpineAnime::IDLE);
            break;
        }
    default:
        this->SetAnime(SpineAnime::IDLE);
        break;
    }
    return TRUE;
}

BOOL Spine::Update(FLOAT elapseTime)
{
    spAnimationState_update(this->animationState, elapseTime);
    spAnimationState_apply(this->animationState, this->skeleton);
    spSkeleton_updateWorldTransform(this->skeleton);

    return TRUE;
}
