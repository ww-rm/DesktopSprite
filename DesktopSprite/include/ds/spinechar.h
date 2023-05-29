#pragma once
#ifndef DS_SPINECHAR_H
#define DS_SPINECHAR_H

#include <ds/framework.h>
#include <ds/utils.h>
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
    FLUSH,

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
    std::list<std::wstring> animationNames;

    int vtPosBufferMaxLen = 0;
    float* vtPosBuffer = NULL;

public:
    Spine();
    ~Spine();

    BOOL CreateResources(PCSTR atlasPath, PCSTR skelPath);
    void DisposeResources();

    // 获取所有存在的动画名字
    std::list<std::wstring>& GetAnimeNames();

    // 获得纹理
    Gdiplus::Bitmap* GetTexture();

    // 设置动画
    void SetAnimation(PCWSTR animationName, BOOL loop = TRUE, INT trackIndex = 0);
    void AddAnimation(PCWSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0);

    // 更新状态
    BOOL Update(FLOAT elapseTime);

    // 获得要渲染的基元
    BOOL GetMeshTriangles(std::vector<VERTEX>& vertexBuffer, std::vector<int>& vertexIndexBuffer);
};

class SpineChar
{
private:
    // 渲染设备数据
    HWND targetWnd = NULL;
    HDC hdcScreen = NULL;
    HDC hdcMem = NULL;
    Gdiplus::Graphics* graphics = NULL;

    // 渲染数据缓冲
    std::vector<int> vertexIndexBuffer;
    std::vector<VERTEX> vertexBuffer;

    // spine 数据
    Spine* spine = NULL;
    Gdiplus::Bitmap* texture = NULL;
    Gdiplus::TextureBrush* textureBrush = NULL;
    BOOL flipX = TRUE;
    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::wstring> animeToName;

    // 主循环数据
    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;
    INT maxFps = 30; // 最大帧率
    FLOAT frameInterval = 1000.0f / 30.0f + 0.5f; // 帧间间隔, 毫秒数


public:
    SpineChar(HWND targetWnd);
    ~SpineChar();

    // 创建与释放绘图资源
    BOOL CreateTargetResourcse();
    void ReleaseTargetResources();

    // 加载与卸载 spine 数据
    BOOL LoadSpine(PCSTR atlasPath, PCSTR skelPath);
    void UnloadSpine();

    void DrawTriangles();
    BOOL RenderFrame();

    // 多线程锁住数据, caller 自行调用
    BOOL Lock();
    BOOL Unlock();

    // 启动/停止运行
    static DWORD CALLBACK FrameProc(_In_ LPVOID lpParameter);
    DWORD Mainloop();

    BOOL Start();
    BOOL Stop();

    BOOL Update(UINT milliseconds);
    BOOL Render();

    // 设置动画
    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

    // 发送一个动作
    BOOL SendAction(SpineAction action);

    // 设置精灵不同动画要使用的 spine 动画
    void SetAnimeName(SpineAnime anime, PCWSTR name);
    PCWSTR GetAnimeName(SpineAnime anime);

    // 获取当前加载的 spine 的所有动画名字, 如果 spine 加载失败则返回 NULL
    const std::list<std::string>* GetAnimeNames();

    // 设置帧率
    void SetMaxFps(INT fps);
    INT GetMaxFps() const;
};


#endif // DS_SPINECHAR_H
