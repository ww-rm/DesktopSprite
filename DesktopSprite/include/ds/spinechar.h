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

    // ��ȡ���д��ڵĶ�������
    std::list<std::string>& GetAnimeNames() { return this->animationNames; };

    // �������
    Gdiplus::Bitmap* GetTexture() { return (Gdiplus::Bitmap*)this->atlas->pages->rendererObject; }

    // ���ö���
    void SetAnimation(PCSTR animationName, BOOL loop = TRUE, INT trackIndex = 0) { spAnimationState_setAnimationByName(this->animationState, trackIndex, animationName, loop); }
    void AddAnimation(PCSTR animationName, BOOL loop = TRUE, FLOAT delay = 0, INT trackIndex = 0) { spAnimationState_addAnimationByName(this->animationState, trackIndex, animationName, loop, delay); }

    // ����״̬
    BOOL Update(FLOAT elapseTime);

    // ���Ҫ��Ⱦ�Ļ�Ԫ
    BOOL GetMeshTriangles(std::vector<VERTEX>& vertexBuffer, std::vector<int>& vertexIndexBuffer);
};

class SpineChar
{
private:
    // ��Ⱦ�豸����
    HWND targetWnd = NULL;
    HDC hdcScreen = NULL;
    HDC hdcMem = NULL;
    Gdiplus::Graphics* graphics = NULL;

    // ��Ⱦ���ݻ���
    std::vector<int> vertexIndexBuffer;
    std::vector<VERTEX> vertexBuffer;

    // spine ����
    Spine* spine = NULL;
    Gdiplus::Bitmap* texture = NULL;
    Gdiplus::TextureBrush* textureBrush = NULL;
    BOOL flipX = TRUE;
    SpineState state = SpineState::IDLE;
    std::map<SpineAnime, std::string> animeToName;

    // ��ѭ������
    HANDLE threadMutex = NULL;
    HANDLE threadEvent = NULL;
    HANDLE thread = NULL;
    HighResolutionTimer timer;
    INT maxFps = 30; // ���֡��
    FLOAT frameInterval = 1000.0f / 30.0f + 0.5f; // ֡����, ������


public:
    SpineChar(HWND targetWnd);
    ~SpineChar();

    // �������ͷŻ�ͼ��Դ
    BOOL CreateTargetResourcse();
    void ReleaseTargetResources();

    // ������ж�� spine ����
    BOOL LoadSpine(PCSTR atlasPath, PCSTR skelPath);
    void UnloadSpine();

    void DrawTriangles();
    BOOL RenderFrame();

    // ���߳���ס����, caller ���е���
    BOOL Lock() { return !WaitForSingleObject(this->threadMutex, INFINITE); }
    BOOL Unlock() { return ReleaseMutex(this->threadMutex); }

    // ����/ֹͣ����
    static DWORD CALLBACK FrameProc(_In_ LPVOID lpParameter) { 
        return ((SpineChar*)lpParameter)->Mainloop(); 
    }
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
    void SetAnimeName(SpineAnime anime, PCSTR name) { this->animeToName[anime] = name; }
    PCSTR GetAnimeName(SpineAnime anime) { return this->animeToName[anime].c_str(); }

    // ����֡��
    void SetMaxFps(INT fps) { this->maxFps = fps; this->frameInterval = 1000.0f / (FLOAT)fps + 0.5f; }
    INT GetMaxFps() const { return this->maxFps; }
};


#endif // DS_SPINECHAR_H
