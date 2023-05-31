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
    // spine ��Դ
    spAtlas* atlas = NULL;
    spSkeletonBinary* skeletonBinary = NULL;
    spSkeletonJson* skeletonJson = NULL;
    spSkeletonData* skeletonData = NULL;
    spAnimationStateData* animationStateData = NULL;
    spSkeleton* skeleton = NULL;
    spAnimationState* animationState = NULL;
    std::list<std::wstring> animationNames;

    // ���㻺����, �ڷ�����ᶯ̬����
    int vtPosBufferMaxLen = 0;
    float* vtPosBuffer = NULL;

    // spine ��������
    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::wstring> animeToName;

public:
    SpineChar();
    ~SpineChar();

    // ������ж�� spine ����
    BOOL LoadSpine(PCWSTR atlasPath, PCWSTR skelPath, UINT scale);
    void UnloadSpine();
    BOOL Loaded() const;

    // �������
    Gdiplus::Bitmap* GetTexture();

    // ����״̬
    BOOL Update(FLOAT elapseTime);

    // ���Ҫ��Ⱦ�Ļ�Ԫ
    BOOL GetMeshTriangles(std::vector<VERTEX>* vertexBuffer, std::vector<int>* vertexIndexBuffer);

    // ����һ������
    BOOL SendAction(SpineAction action);

    // ����ʵʱ����
    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

    // ��ȡ��ǰ���ص� spine �����ж�������, ��� spine ����ʧ���򷵻� NULL
    const std::list<std::wstring>* GetAnimeNames();

    // ���þ��鲻ͬ����Ҫʹ�õ� spine ����
    void SetAnimeName(SpineAnime anime, PCWSTR name);
    PCWSTR GetAnimeName(SpineAnime anime);

    BOOL SetFlipX(BOOL flip);
    BOOL GetFlipX(BOOL* flip) const;

    BOOL SetPosition(FLOAT x, FLOAT y);
    BOOL GetPosition(FLOAT* x, FLOAT* y) const;

private:
    // ���� animationState ����
    BOOL SetAnimation(PCWSTR animationName, BOOL loop = TRUE, INT trackIndex = 0);
    BOOL AddAnimation(PCWSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0);
};

class SpineRenderer
{
private:
    // ��Ⱦ�豸����
    HWND targetWnd = NULL;
    HDC hdcScreen = NULL;
    HDC hdcMem = NULL;
    RECT rcTarget = { 0 };
    Gdiplus::Graphics* graphics = NULL;

    // ��Ⱦ���ݻ���
    std::vector<int> vertexIndexBuffer;
    std::vector<VERTEX> vertexBuffer;

    // �� spine �йص���Դ
    SpineChar* spinechar = NULL;
    Gdiplus::Bitmap* texture = NULL;
    INT texWidth = 0;
    INT texHeight = 0;
    Gdiplus::TextureBrush* textureBrush = NULL;

    // ��ѭ������
    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;

    // ��Ⱦ����
    INT maxFps = 30; // ���֡��
    FLOAT frameInterval = 1000.0f / 30.0f; // ֡����, ������
    BYTE transparency = 255;

public:
    SpineRenderer(HWND targetWnd, SpineChar* spinechar);

    // �������ͷŻ�ͼ��Դ
    BOOL CreateTargetResourcse();
    void ReleaseTargetResources();

    BOOL CreateSpineResources();
    void ReleaseSpineResources();

    // ���߳���ס����, caller ���е���
    BOOL Lock();
    BOOL Unlock();

    // ����/ֹͣ����
    BOOL Start();
    BOOL Stop();
    BOOL IsRendering() const;

    // ����֡��
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
