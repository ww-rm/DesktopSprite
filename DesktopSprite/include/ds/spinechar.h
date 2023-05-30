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
    spSkeletonJson* skeletonJson = NULL;
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

    BOOL CreateResources(PCWSTR atlasPath, PCWSTR skelPath);
    void DisposeResources();

    // ��ȡ���д��ڵĶ�������
    const std::list<std::wstring>* GetAnimeNames();

    // �������
    Gdiplus::Bitmap* GetTexture();

    // ���ö���
    void SetAnimation(PCWSTR animationName, BOOL loop = TRUE, INT trackIndex = 0);
    void AddAnimation(PCWSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0);

    // ����״̬
    BOOL Update(FLOAT elapseTime);

    // ���Ҫ��Ⱦ�Ļ�Ԫ
    BOOL GetMeshTriangles(std::vector<VERTEX>* vertexBuffer, std::vector<int>* vertexIndexBuffer);
};

class SpineChar
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

    // spine ����
    Spine* spine = NULL; // ����ʧ�ܵ�ʱ���ǿ�ָ��
    Gdiplus::Bitmap* texture = NULL;
    INT texWidth = 0;
    INT texHeight = 0;
    Gdiplus::TextureBrush* textureBrush = NULL;
    // BOOL flipX = FALSE; TODO?
    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::wstring> animeToName;

    // ��ѭ������
    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;

    // ��Ⱦ����
    INT maxFps = 30; // ���֡��
    FLOAT frameInterval = 1000.0f / 30.0f + 0.5f; // ֡����, ������
    BYTE transparency = 0xFF;
    FLOAT scale = 100.0f;


public:
    SpineChar(HWND targetWnd);
    ~SpineChar();

    // �������ͷŻ�ͼ��Դ
    BOOL CreateTargetResourcse();
    void ReleaseTargetResources();

    // ������ж�� spine ����
    BOOL LoadSpine(PCWSTR atlasPath, PCWSTR skelPath);
    void UnloadSpine();

    void DrawTriangles();
    BOOL RenderFrame();

    // ���߳���ס����, caller ���е���
    BOOL Lock();
    BOOL Unlock();

    // ����/ֹͣ����
    static DWORD CALLBACK FrameProc(_In_ LPVOID lpParameter);
    DWORD Mainloop();

    BOOL Start();
    BOOL Stop();

    BOOL Update(UINT milliseconds);
    BOOL Render();

    // ���ö���
    BOOL SetAnime(SpineAnime anime, BOOL isOneShot = FALSE, SpineAnime rollin = SpineAnime::IDLE);

    // ����һ������
    BOOL SendAction(SpineAction action);

    // ���þ��鲻ͬ����Ҫʹ�õ� spine ����
    void SetAnimeName(SpineAnime anime, PCWSTR name);
    PCWSTR GetAnimeName(SpineAnime anime);

    // ��ȡ��ǰ���ص� spine �����ж�������, ��� spine ����ʧ���򷵻� NULL
    const std::list<std::wstring>* GetAnimeNames();

    // ����֡��
    void SetMaxFps(INT fps);
    INT GetMaxFps() const;

    void SetTransparency(UINT transparency);
    UINT GetTransparency() const;

    BOOL SetScale(UINT scale);
    UINT GetScale() const;
};


#endif // DS_SPINECHAR_H
