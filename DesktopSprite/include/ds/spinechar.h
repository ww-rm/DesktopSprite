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
    REFRESH,

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

class SpineChar
{
private:
    // spine 资源
    spAtlas* atlas = NULL;
    spSkeletonBinary* skeletonBinary = NULL;
    spSkeletonJson* skeletonJson = NULL;
    spSkeletonData* skeletonData = NULL;
    spAnimationStateData* animationStateData = NULL;
    spSkeleton* skeleton = NULL;
    spAnimationState* animationState = NULL;
    std::list<std::wstring> animationNames;

    // 顶点缓冲区, 在方法里会动态扩容
    int vtPosBufferMaxLen = 0;
    float* vtPosBuffer = NULL;

    // spine 动画数据
    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::wstring> animeToName;

public:
    SpineChar();
    ~SpineChar();

    // 加载与卸载 spine 数据
    BOOL LoadSpine(PCWSTR atlasPath, PCWSTR skelPath, UINT scale);
    void UnloadSpine();
    BOOL Loaded() const;

    // 获得纹理
    PCWSTR GetTexture();

    // 更新状态
    BOOL Update(FLOAT elapseTime);

    // 获得要渲染的基元
    BOOL GetMeshTriangles(std::vector<VERTEX>* vertexBuffer, std::vector<int>* vertexIndexBuffer);

    // 发送一个动作
    BOOL SendAction(SpineAction action);

    // 设置实时动画
    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

    // 获取当前加载的 spine 的所有动画名字, 如果 spine 加载失败则返回 NULL
    const std::list<std::wstring>* GetAnimeNames();

    // 设置精灵不同动画要使用的 spine 动画
    void SetAnimeName(SpineAnime anime, PCWSTR name);
    PCWSTR GetAnimeName(SpineAnime anime);

    BOOL SetFlipX(BOOL flip);
    BOOL GetFlipX(BOOL* flip) const;

    BOOL SetPosition(FLOAT x, FLOAT y);
    BOOL GetPosition(FLOAT* x, FLOAT* y) const;

private:
    // 设置 animationState 动画
    BOOL SetAnimation(PCWSTR animationName, BOOL loop = TRUE, INT trackIndex = 0);
    BOOL AddAnimation(PCWSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0);
};

class SpineRenderer
{
private:
    ID2D1Factory* pD2DFactory = NULL;
    IWICImagingFactory* pWICFactory = NULL;

    // 渲染设备数据
    HWND targetWnd = NULL;
    HDC hdcScreen = NULL;
    HDC hdcMem = NULL;
    RECT rcTarget = { 0 };
    ID2D1DCRenderTarget* pDCrenderTarget = NULL;

    // 渲染数据缓冲
    std::vector<int> vertexIndexBuffer;
    std::vector<VERTEX> vertexBuffer;
    std::vector<ID2D1Mesh*> trianglesBuffer;

    // 与 spine 有关的资源
    SpineChar* spinechar = NULL;
    ID2D1Bitmap* texture = NULL;
    INT texWidth = 0;
    INT texHeight = 0;
    ID2D1BitmapBrush* textureBrush = NULL;

    // 主循环数据
    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;

    // 渲染参数
    INT maxFps = 30; // 最大帧率
    FLOAT frameInterval = 1000.0f / 30.0f; // 帧间间隔, 毫秒数
    BYTE transparency = 255;

public:
    SpineRenderer(HWND targetWnd, SpineChar* spinechar);
    ~SpineRenderer();

    // 创建与释放绘图资源
    BOOL CreateTargetResources();
    void ReleaseTargetResources();
    ID2D1Bitmap* CreateBitmapFromFile(PCWSTR path);

    BOOL CreateSpineResources();
    void ReleaseSpineResources();

    // 多线程锁住数据, caller 自行调用
    BOOL Lock();
    BOOL Unlock();

    // 启动/停止运行
    BOOL Start();
    BOOL Stop();
    BOOL IsRendering() const;

    // 设置帧率
    void SetMaxFps(INT fps);
    INT GetMaxFps() const;

    void SetTransparency(UINT transparency);
    UINT GetTransparency() const;

private:
    void DrawTriangles();
    BOOL RenderFrame();

    static DWORD CALLBACK FrameProc(_In_ LPVOID lpParameter);
    DWORD Mainloop();

    BOOL Update(UINT milliseconds);
    BOOL Render();

};

#endif // DS_SPINECHAR_H
