#include <ds/framework.h>

#include <ds/utils.h>
#include <spine/spine.h>

#include <ds/spinechar.h>

// [XY](3, 2) = [UV](3, 3) @ M(3, 2)
// | x1, y1 |   | u1, v1, 1 |   | m11, m12 |
// | x2, y2 | = | u2, v2, 1 | @ | m21, m22 |
// | x3, y3 |   | u3, v3, 1 |   | dx,  dy  |
void GetAffineMatrix(
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    float u1, float v1,
    float u2, float v2,
    float u3, float v3,
    Gdiplus::Matrix* m
)
{
    float x12 = x1 - x2;
    float y12 = y1 - y2;
    float x23 = x2 - x3;
    float y23 = y2 - y3;
    float u12 = u1 - u2;
    float v12 = v1 - v2;
    float u23 = u2 - u3;
    float v23 = v2 - v3;

    float invdet = 1.0000f / (u12 * v23 - v12 * u23);
    float m11 = invdet * (x12 * v23 - v12 * x23);
    float m12 = invdet * (y12 * v23 - v12 * y23);
    float m21 = invdet * (u12 * x23 - x12 * u23);
    float m22 = invdet * (u12 * y23 - y12 * u23);
    float dx = x1 - m11 * u1 - m21 * v1;
    float dy = y1 - m12 * u1 - m22 * v1;

    m->SetElements((float)m11, (float)m12, (float)m21, (float)m22, (float)dx, (float)dy);
}

Spine::Spine()
{
    this->vtPosBuffer = new float[2048];
    this->vtPosBufferMaxLen = 2048;
}

Spine::~Spine()
{
    if (this->vtPosBuffer)
    {
        delete[] this->vtPosBuffer;
        this->vtPosBufferMaxLen = 0;
    }
}

BOOL Spine::CreateResources(PCSTR atlasPath, PCSTR skelPath)
{
    if (!(this->atlas =              spAtlas_createFromFile(atlasPath, 0)))                                  return FALSE;
    if (!(this->skeletonBinary =     spSkeletonBinary_create(this->atlas)))                                  return FALSE;
    if (!(this->skeletonData =       spSkeletonBinary_readSkeletonDataFile(this->skeletonBinary, skelPath))) return FALSE;
    if (!(this->animationStateData = spAnimationStateData_create(this->skeletonData)))                       return FALSE;
    if (!(this->skeleton =           spSkeleton_create(this->skeletonData)))                                 return FALSE;
    if (!(this->animationState =     spAnimationState_create(this->animationStateData)))                     return FALSE;

    animationStateData->defaultMix = 0.15f;

    for (int i = 0; i < this->skeletonData->animationsCount; i++)
    {
        this->animationNames.push_back(this->skeletonData->animations[i]->name);
    }

    this->skeleton->x = 350;
    this->skeleton->y = 350;
    this->skeleton->flipX = 1;
    this->skeleton->flipY = 1;

    return TRUE;
}

void Spine::DisposeResources()
{
    if (this->animationState) 
    {
        spAnimationState_dispose(this->animationState);
        this->animationState = NULL;
    }
    if (this->skeleton) 
    {
        spSkeleton_dispose(this->skeleton);
        this->skeleton = NULL;
    }
    if (this->animationStateData) 
    {
        spAnimationStateData_dispose(this->animationStateData);
        this->animationStateData = NULL;
    }
    if (this->skeletonData) 
    {
        spSkeletonData_dispose(this->skeletonData);
        this->skeletonData = NULL;
    }
    if (this->skeletonBinary) 
    {
        spSkeletonBinary_dispose(this->skeletonBinary);
        this->skeletonBinary = NULL;
    }
    if (this->atlas) 
    {
        spAtlas_dispose(this->atlas);
        this->atlas = NULL;
    }
}

std::list<std::string>& Spine::GetAnimeNames()
{
    return this->animationNames;
}

Gdiplus::Bitmap* Spine::GetTexture()
{
    return (Gdiplus::Bitmap*)this->atlas->pages->rendererObject;
}

void Spine::SetAnimation(PCWSTR animationName, BOOL loop, INT trackIndex)
{
    char animationNameA[MAX_PATH] = { 0 };
    StrWtoA(animationName, animationNameA, MAX_PATH);
    spAnimationState_setAnimationByName(this->animationState, trackIndex, animationNameA, loop);
}

void Spine::AddAnimation(PCWSTR animationName, BOOL loop, FLOAT delay, INT trackIndex)
{
    char animationNameA[MAX_PATH] = { 0 };
    StrWtoA(animationName, animationNameA, MAX_PATH);
    spAnimationState_addAnimationByName(this->animationState, trackIndex, animationNameA, loop, delay);
}

BOOL Spine::Update(FLOAT elapseTime)
{
    spAnimationState_update(this->animationState, elapseTime);
    spAnimationState_apply(this->animationState, this->skeleton);
    spSkeleton_updateWorldTransform(this->skeleton);

    return TRUE;
}

BOOL Spine::GetMeshTriangles(std::vector<VERTEX>& vertexBuffer, std::vector<int>& vertexIndexBuffer)
{
    vertexBuffer.clear();
    vertexIndexBuffer.clear();

    for (int i = 0; i < this->skeleton->slotsCount; i++) {
        spSlot* slot = skeleton->drawOrder[i];
        spAttachment* attachment = slot->attachment;

        if (!attachment)
        {
            continue;
        }

        int preVertexCount = (int)vertexBuffer.size();
        if (attachment->type == SP_ATTACHMENT_REGION)
        {
            spRegionAttachment* region = (spRegionAttachment*)attachment;

            float tintR = skeleton->color.r * slot->color.r * region->color.r;
            float tintG = skeleton->color.g * slot->color.g * region->color.g;
            float tintB = skeleton->color.b * slot->color.b * region->color.b;
            float tintA = skeleton->color.a * slot->color.a * region->color.a;

            spRegionAttachment_computeWorldVertices(region, slot->bone, this->vtPosBuffer, 0, 2);

            // push vt_0 -> vt_1 -> vt_2 -> vt_3
            vertexBuffer.push_back(
                { this->vtPosBuffer[0], this->vtPosBuffer[1],
                region->uvs[0], region->uvs[1],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer.push_back(
                { this->vtPosBuffer[2], this->vtPosBuffer[3],
                region->uvs[2], region->uvs[3],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer.push_back(
                { this->vtPosBuffer[4], this->vtPosBuffer[5],
                region->uvs[4], region->uvs[5],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer.push_back(
                { this->vtPosBuffer[6], this->vtPosBuffer[7],
                region->uvs[6], region->uvs[7],
                tintR, tintG, tintB, tintA }
            );

            // split to two triangles
            vertexIndexBuffer.push_back(0 + preVertexCount);
            vertexIndexBuffer.push_back(1 + preVertexCount);
            vertexIndexBuffer.push_back(2 + preVertexCount);

            vertexIndexBuffer.push_back(2 + preVertexCount);
            vertexIndexBuffer.push_back(3 + preVertexCount);
            vertexIndexBuffer.push_back(0 + preVertexCount);
        }
        else if (attachment->type == SP_ATTACHMENT_MESH)
        {
            spMeshAttachment* mesh = (spMeshAttachment*)attachment;
            float tintR = skeleton->color.r * slot->color.r * mesh->color.r;
            float tintG = skeleton->color.g * slot->color.g * mesh->color.g;
            float tintB = skeleton->color.b * slot->color.b * mesh->color.b;
            float tintA = skeleton->color.a * slot->color.a * mesh->color.a;

            // realloc
            if (mesh->super.worldVerticesLength > this->vtPosBufferMaxLen)
            {
                int largerLen = mesh->super.worldVerticesLength * 2;
                float* largerBuffer = new float[largerLen];
                CopyMemory(largerBuffer, this->vtPosBuffer, this->vtPosBufferMaxLen);
                delete[] this->vtPosBuffer;
                this->vtPosBufferMaxLen = largerLen;
            }

            spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength, this->vtPosBuffer, 0, 2);

            // add vertex
            for (int j = 0; j < mesh->super.worldVerticesLength; j += 2)
            {
                vertexBuffer.push_back(
                    { this->vtPosBuffer[j], this->vtPosBuffer[j + 1],
                    mesh->uvs[j], mesh->uvs[j + 1],
                    tintR, tintG, tintB, tintA, }
                );
            }

            // add index
            for (int j = 0; j < mesh->trianglesCount; j++) 
            {
                vertexIndexBuffer.push_back(mesh->triangles[j] + preVertexCount);
            }
        }
        else
        {
            continue;
        }
    }
    return TRUE;
}


SpineChar::SpineChar(HWND targetWnd) : targetWnd(targetWnd)
{
    this->threadEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!this->threadEvent)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(-1);
    }

    this->threadMutex = CreateMutexW(NULL, FALSE, NULL);
    if (!this->threadMutex)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        exit(-1);
    }

    // continuous
    this->animeToName[SpineAnime::IDLE] = L"normal";
    this->animeToName[SpineAnime::DRAG] = L"tuozhuai";
    this->animeToName[SpineAnime::WORK] = L"walk";
    this->animeToName[SpineAnime::SLEEP] = L"sleep";

    // one-shot
    this->animeToName[SpineAnime::STAND] = L"stand";
    this->animeToName[SpineAnime::TOUCH] = L"touch";
    this->animeToName[SpineAnime::WINK] = L"motou";
    this->animeToName[SpineAnime::VICTORY] = L"victory";
    this->animeToName[SpineAnime::DANCE] = L"dance";
    this->animeToName[SpineAnime::DIZZY] = L"yun";

    this->vertexBuffer.reserve(2048);
    this->vertexIndexBuffer.reserve(2048);
}

SpineChar::~SpineChar()
{
    this->UnloadSpine();

    if (this->threadMutex)
    {
        CloseHandle(this->threadMutex);
        this->threadMutex = NULL;
    }

    if (this->threadEvent)
    {
        CloseHandle(this->threadEvent);
        this->threadEvent = NULL;
    }
}

BOOL SpineChar::CreateTargetResourcse()
{
    if (!(this->hdcScreen = GetDC(NULL))) return FALSE;
    if (!(this->hdcMem = CreateCompatibleDC(this->hdcScreen))) return FALSE;

    SIZE screen = { 0 };
    GetScreenResolution(&screen);
    auto r1 = CreateCompatibleBitmap(this->hdcScreen, screen.cx, screen.cy);
    auto r2 = SelectObject(this->hdcMem, r1);
    auto r3 = DeleteObject(r2);

    this->graphics = new Gdiplus::Graphics(this->hdcMem);
    return TRUE;
}

void SpineChar::ReleaseTargetResources()
{
    delete this->graphics;
    DeleteDC(this->hdcMem);
    ReleaseDC(NULL, this->hdcScreen);
}

void SpineChar::DrawTriangles()
{
    int width = this->texture->GetWidth();
    int height = this->texture->GetHeight();
    Gdiplus::Matrix transform;

    for (auto it = this->vertexIndexBuffer.begin(); it != this->vertexIndexBuffer.end(); it += 3)
    {
        VERTEX vt1 = this->vertexBuffer[*it];
        VERTEX vt2 = this->vertexBuffer[*(it + 1)];
        VERTEX vt3 = this->vertexBuffer[*(it + 2)];

        GetAffineMatrix(
            vt1.x, vt1.y, vt2.x, vt2.y, vt3.x, vt3.y,
            vt1.u * width, vt1.v * height, vt2.u * width, vt2.v * height, vt3.u * width, vt3.v * height,
            &transform
        );

        this->textureBrush->SetTransform(&transform);
        Gdiplus::PointF pts[3] = { {vt1.x, vt1.y}, {vt2.x, vt2.y}, {vt3.x, vt3.y} };
        this->graphics->FillPolygon(this->textureBrush, pts, 3);
    }
}

BOOL SpineChar::RenderFrame()
{
    POINT ptSrc = { 0, 0 };
    SIZE sizeWnd = { 1920, 1080 };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    return UpdateLayeredWindow(this->targetWnd, this->hdcScreen, NULL, &sizeWnd, this->hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);
}

BOOL SpineChar::Lock()
{
    return !WaitForSingleObject(this->threadMutex, INFINITE);
}

BOOL SpineChar::Unlock()
{
    return ReleaseMutex(this->threadMutex);
}

BOOL SpineChar::LoadSpine(PCSTR atlasPath, PCSTR skelPath)
{
    Spine* spine = new Spine;
    if (spine->CreateResources(atlasPath, skelPath))
    {
        this->UnloadSpine();
        this->spine = spine;

        for (auto it = this->spine->GetAnimeNames().begin(); it != this->spine->GetAnimeNames().end(); it++)
        {
            OutputDebugStringA((*it).c_str());
            OutputDebugStringA("; ");
        }
        OutputDebugStringA("\n");

        this->spine->SetAnimation(this->animeToName[SpineAnime::IDLE].c_str());
        this->texture = this->spine->GetTexture();
        this->textureBrush = new Gdiplus::TextureBrush(this->texture); // ����һ��
        return TRUE;
    }

    delete spine;
    return FALSE;
}

void SpineChar::UnloadSpine()
{
    if (this->textureBrush)
    {
        delete this->textureBrush;
        this->textureBrush = NULL;
    }
    this->texture = NULL;
    if (this->spine)
    {
        this->spine->DisposeResources();
        delete this->spine;
        this->spine = NULL;
    }
}

DWORD CALLBACK SpineChar::FrameProc(_In_ LPVOID lpParameter) 
{
    return ((SpineChar*)lpParameter)->Mainloop();
}

DWORD SpineChar::Mainloop()
{
    FLOAT elapse = 0;
    this->timer.Start();

    while (WaitForSingleObject(this->threadEvent, 0))
    {
        this->Lock(); // ����

        // ��ʵ�߼�֡���
        elapse = this->timer.GetMilliseconds();

        // ���ü�ʱ��
        this->timer.Start();

        // ����ʵ������²���Ⱦ
        this->Update((UINT)(elapse + 0.5));
        this->Render();

        // �����һ֡����ʵ���
        elapse = this->timer.GetMilliseconds();
        if (elapse < this->frameInterval)
        {
            // �������֡��
            HighResolutionSleep((DWORD)(this->frameInterval - elapse));
        }

        this->Unlock(); // ����
    }

    this->timer.Stop();
    return EXIT_SUCCESS;
}

BOOL SpineChar::Start()
{
    if (!ResetEvent(this->threadEvent))
    {
        return FALSE;
    }

    this->thread = DefCreateThread(SpineChar::FrameProc, (LPVOID)this);
    if (!this->thread)
    {
        ShowLastError(__FUNCTIONW__, __LINE__);
        return FALSE;
    }
    return TRUE;
}

BOOL SpineChar::Stop()
{
    if (!SetEvent(this->threadEvent))
    {
        return FALSE;
    }

    if (this->thread)
    {
        if (WaitForSingleObject(this->thread, INFINITE) || !CloseHandle(this->thread))
        {
            ShowLastError(__FUNCTIONW__, __LINE__);
            return FALSE;
        }
        this->thread = NULL;
    }

    return TRUE;
}

BOOL SpineChar::Update(UINT milliseconds)
{
    return this->spine->Update((FLOAT)milliseconds / 1000.0f);
}

BOOL SpineChar::Render()
{
    this->graphics->Clear(Gdiplus::Color::Transparent);
    this->spine->GetMeshTriangles(this->vertexBuffer, this->vertexIndexBuffer);
    this->DrawTriangles();
    this->RenderFrame();
    return TRUE;
}

BOOL SpineChar::SetAnime(SpineAnime anime, BOOL isOneShot, SpineAnime rollin)
{
    if (!this->spine)
        return FALSE;

    this->spine->SetAnimation(this->animeToName[anime].c_str());
    if (isOneShot)
    {
        this->spine->AddAnimation(this->animeToName[rollin].c_str());
    }
    return TRUE;
}

BOOL SpineChar::SendAction(SpineAction action)
{
    if (!this->spine)
        return FALSE;
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
            break;
        }
    case SpineState::WORK:
        switch (action)
        {
        case SpineAction::ENDWORK:
            this->state = SpineState::IDLE;
            this->SetAnime(SpineAnime::VICTORY, TRUE);
            break;

        case SpineAction::TOUCH:
            this->SetAnime(SpineAnime::TOUCH, TRUE, SpineAnime::WORK);
            break;
        case SpineAction::DIZZY:
            this->SetAnime(SpineAnime::DIZZY, TRUE, SpineAnime::WORK);
            break;
        default:
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
        case SpineAction::TOUCH:
            this->state = SpineState::IDLE;
            this->SetAnime(SpineAnime::TOUCH, TRUE);
            break;
        default:
            break;
        }
    default:
        break;
    }
    return TRUE;
}

void SpineChar::SetAnimeName(SpineAnime anime, PCWSTR name)
{
    this->animeToName[anime] = name;
}

PCWSTR SpineChar::GetAnimeName(SpineAnime anime)
{
    return this->animeToName[anime].c_str();
}

void SpineChar::SetMaxFps(INT fps)
{
    this->maxFps = fps; 
    this->frameInterval = 1000.0f / (FLOAT)fps + 0.5f;
}

INT SpineChar::GetMaxFps() const
{
    return this->maxFps;
}
