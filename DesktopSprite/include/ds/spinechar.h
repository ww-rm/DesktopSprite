#pragma once
#ifndef DS_SPINECHAR_H
#define DS_SPINECHAR_H

#include <ds/framework.h>
#include <spine/spine.h>

typedef struct _VERTEX {
    float x, y;
    float u, v;
    float r, g, b, a;
} VERTEX;

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
    spAtlas* atlas = NULL;
    spSkeletonBinary* skeletonBinary = NULL;
    spSkeletonData* skeletonData = NULL;
    spAnimationStateData* animationStateData = NULL;
    spSkeleton* skeleton = NULL;
    spAnimationState* animationState = NULL;
    std::list<std::string> animationNames;

    int vtPosBufferMaxLen = 0;
    float* vtPosBuffer = NULL;

public:
    Spine();
    ~Spine();

    BOOL CreateResources(PCSTR atlasPath, PCSTR skelPath);
    void DisposeResources();

    // 获取所有存在的动画名字
    std::list<std::string>& GetAnimeNames() { return this->animationNames; };

    // 获得纹理
    Gdiplus::Bitmap* GetTexture() { return (Gdiplus::Bitmap*)this->atlas->pages->rendererObject; }

    // 设置动画
    void SetAnimation(PCSTR animationName, BOOL loop = TRUE, INT trackIndex = 0) { spAnimationState_setAnimationByName(this->animationState, trackIndex, animationName, loop); }
    void AddAnimation(PCSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0) { spAnimationState_addAnimationByName(this->animationState, trackIndex, animationName, loop, delay); }

    // 更新状态
    BOOL Update(FLOAT elapseTime);

    // 获得要渲染的基元
    BOOL GetRenderTriangles(std::vector<VERTEX>& vertexBuffer, std::vector<int>& vertexIndexBuffer);
};

class SpineChar
{
private:
    HWND targetWnd = NULL;
    Gdiplus::Graphics* graphics = NULL;

    Spine* spine = NULL;
    Gdiplus::Bitmap* texture = NULL;
    Gdiplus::TextureBrush* textureBrush = NULL;

    INT maxFps = 30; // 最大帧率
    FLOAT frameInterval = 1000.0f / 30.0f + 0.5f; // 帧间间隔, 毫秒数

    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;

    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::string> animeToName;

    std::vector<int> vertexIndexBuffer;
    std::vector<VERTEX> vertexBuffer;

public:
    SpineChar(HWND targetWnd);
    ~SpineChar();

    void RenderTriangles();

    // 多线程锁住数据, caller 自行调用
    BOOL Lock() { return !WaitForSingleObject(this->threadMutex, INFINITE); }
    BOOL Unlock() { return ReleaseMutex(this->threadMutex); }

    // 启动/停止运行
    static DWORD CALLBACK FrameProc(_In_ LPVOID lpParameter) { return ((SpineChar*)lpParameter)->Mainloop(); }
    DWORD Mainloop();
    BOOL Start();
    BOOL Stop();

    BOOL Update(UINT milliseconds);
    BOOL Render();

    // 加载与卸载 spine 数据
    BOOL LoadSpine(PCSTR atlasPath, PCSTR skelPath);
    void UnloadSpine();

    // 设置动画
    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

    // 发送一个动作
    BOOL SendAction(SpineAction action);

    // 设置精灵不同动画要使用的 spine 动画
    void SetAnimeName(SpineAnime anime, PCSTR name) { this->animeToName[anime] = name; }
    PCSTR GetAnimeName(SpineAnime anime) { return this->animeToName[anime].c_str(); }

    // 设置帧率
    void SetMaxFps(INT fps) { this->maxFps = fps; this->frameInterval = 1000.0f / (FLOAT)fps + 0.5f; }
    INT GetMaxFps() const { return this->maxFps; }
};


#endif // DS_SPINECHAR_H
