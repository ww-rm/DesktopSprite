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
        this->vtPosBuffer = NULL;
        this->vtPosBufferMaxLen = 0;
    }
}

BOOL Spine::CreateResources(PCWSTR atlasPath, PCWSTR skelPath)
{
    CHAR pathBuffer[MAX_PATH] = { 0 };

    StrWtoA(atlasPath, pathBuffer, MAX_PATH);
    if (!(this->atlas = spAtlas_createFromFile(pathBuffer, 0)))
        return FALSE;

    StrWtoA(skelPath, pathBuffer, MAX_PATH);
    PCWSTR skelExtension = PathFindExtensionW(skelPath);

    // 如果是 json 格式
    if (skelExtension && !StrCmpW(skelExtension, L".json"))
    {
        if (!(this->skeletonJson = spSkeletonJson_create(this->atlas)))
            return FALSE;
        if (!(this->skeletonData = spSkeletonJson_readSkeletonDataFile(this->skeletonJson, pathBuffer)))
            return FALSE;
    }
    else
    {
        if (!(this->skeletonBinary = spSkeletonBinary_create(this->atlas)))                                  
            return FALSE;
        if (!(this->skeletonData = spSkeletonBinary_readSkeletonDataFile(this->skeletonBinary, pathBuffer)))
            return FALSE;
    }
    if (!(this->animationStateData = spAnimationStateData_create(this->skeletonData)))                       
        return FALSE;
    if (!(this->skeleton = spSkeleton_create(this->skeletonData)))                                 
        return FALSE;
    if (!(this->animationState = spAnimationState_create(this->animationStateData)))                     
        return FALSE;

    animationStateData->defaultMix = 0.15f;

    WCHAR strBuffer[MAX_PATH] = { 0 };
    for (int i = 0; i < this->skeletonData->animationsCount; i++)
    {
        StrAtoW(this->skeletonData->animations[i]->name, strBuffer, MAX_PATH);
        this->animationNames.push_back(strBuffer);
    }

    this->skeleton->x = 0;
    this->skeleton->y = 0;
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
    if (this->skeletonJson)
    {
        spSkeletonJson_dispose(this->skeletonJson);
        this->skeletonJson = NULL;
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

const std::list<std::wstring>* Spine::GetAnimeNames()
{
    return &this->animationNames;
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

BOOL Spine::GetMeshTriangles(std::vector<VERTEX>* vertexBuffer, std::vector<int>* vertexIndexBuffer)
{
    for (int i = 0; i < this->skeleton->slotsCount; i++) {
        spSlot* slot = skeleton->drawOrder[i];
        spAttachment* attachment = slot->attachment;

        if (!attachment)
        {
            continue;
        }

        int preVertexCount = (int)vertexBuffer->size();
        if (attachment->type == SP_ATTACHMENT_REGION)
        {
            spRegionAttachment* region = (spRegionAttachment*)attachment;

            float tintR = skeleton->color.r * slot->color.r * region->color.r;
            float tintG = skeleton->color.g * slot->color.g * region->color.g;
            float tintB = skeleton->color.b * slot->color.b * region->color.b;
            float tintA = skeleton->color.a * slot->color.a * region->color.a;

            spRegionAttachment_computeWorldVertices(region, slot->bone, this->vtPosBuffer, 0, 2);

            // push vt_0 -> vt_1 -> vt_2 -> vt_3
            vertexBuffer->push_back(
                { this->vtPosBuffer[0], this->vtPosBuffer[1],
                region->uvs[0], region->uvs[1],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer->push_back(
                { this->vtPosBuffer[2], this->vtPosBuffer[3],
                region->uvs[2], region->uvs[3],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer->push_back(
                { this->vtPosBuffer[4], this->vtPosBuffer[5],
                region->uvs[4], region->uvs[5],
                tintR, tintG, tintB, tintA }
            );

            vertexBuffer->push_back(
                { this->vtPosBuffer[6], this->vtPosBuffer[7],
                region->uvs[6], region->uvs[7],
                tintR, tintG, tintB, tintA }
            );

            // split to two triangles
            vertexIndexBuffer->push_back(0 + preVertexCount);
            vertexIndexBuffer->push_back(1 + preVertexCount);
            vertexIndexBuffer->push_back(2 + preVertexCount);

            vertexIndexBuffer->push_back(2 + preVertexCount);
            vertexIndexBuffer->push_back(3 + preVertexCount);
            vertexIndexBuffer->push_back(0 + preVertexCount);
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
                this->vtPosBuffer = largerBuffer;
                this->vtPosBufferMaxLen = largerLen;
            }

            spVertexAttachment_computeWorldVertices(SUPER(mesh), slot, 0, mesh->super.worldVerticesLength, this->vtPosBuffer, 0, 2);

            // add vertex
            for (int j = 0; j < mesh->super.worldVerticesLength; j += 2)
            {
                vertexBuffer->push_back(
                    { this->vtPosBuffer[j], this->vtPosBuffer[j + 1],
                    mesh->uvs[j], mesh->uvs[j + 1],
                    tintR, tintG, tintB, tintA, }
                );
            }

            // add index
            for (int j = 0; j < mesh->trianglesCount; j++) 
            {
                vertexIndexBuffer->push_back(mesh->triangles[j] + preVertexCount);
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
    if (!(this->hdcScreen = GetDC(NULL))) 
        return FALSE;
    if (!(this->hdcMem = CreateCompatibleDC(this->hdcScreen))) 
        return FALSE;

    GetWindowRect(this->targetWnd, &this->rcTarget);
    DeleteObject(
        SelectObject(
            this->hdcMem, 
            CreateCompatibleBitmap(this->hdcScreen, this->rcTarget.right - this->rcTarget.left, this->rcTarget.bottom - this->rcTarget.top)
        )
    );

    this->graphics = new Gdiplus::Graphics(this->hdcMem);
    return TRUE;
}

void SpineChar::ReleaseTargetResources()
{
    if (this->graphics)
    {
        delete this->graphics;
        this->graphics = NULL;
    }
    if (this->hdcMem)
    {
        DeleteDC(this->hdcMem);
        this->hdcMem = NULL;
    }
    if (this->hdcScreen)
    {
        ReleaseDC(NULL, this->hdcScreen);
        this->hdcScreen = NULL;
    }
}

void SpineChar::DrawTriangles()
{
    int W = this->texWidth;
    int H = this->texHeight;
    Gdiplus::Matrix transform;
    VERTEX* vt1 = NULL;
    VERTEX* vt2 = NULL;
    VERTEX* vt3 = NULL;

    for (auto it = this->vertexIndexBuffer.begin(); it != this->vertexIndexBuffer.end(); it += 3)
    {
        vt1 = &this->vertexBuffer[*it];
        vt2 = &this->vertexBuffer[*(it + 1)];
        vt3 = &this->vertexBuffer[*(it + 2)];

        GetAffineMatrix(
            vt1->x, vt1->y, 
            vt2->x, vt2->y, 
            vt3->x, vt3->y, 
            vt1->u * W, vt1->v * H, 
            vt2->u * W, vt2->v * H, 
            vt3->u * W, vt3->v * H, 
            &transform
        );

        this->textureBrush->SetTransform(&transform);
        Gdiplus::PointF pts[3] = { {vt1->x, vt1->y}, {vt2->x, vt2->y}, {vt3->x, vt3->y} };
        this->graphics->FillPolygon(this->textureBrush, pts, 3);
    }
}

BOOL SpineChar::RenderFrame()
{
    POINT ptSrc = { 0, 0 };
    SIZE sizeWnd = { 
        this->rcTarget.right - this->rcTarget.left, 
        this->rcTarget.bottom - this->rcTarget.top
    };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, this->transparency, AC_SRC_ALPHA};
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

BOOL SpineChar::LoadSpine(PCWSTR atlasPath, PCWSTR skelPath)
{
    this->spine = new Spine();
    if (spine->CreateResources(atlasPath, skelPath))
    {
        for (auto it = this->spine->GetAnimeNames()->begin(); it != this->spine->GetAnimeNames()->end(); it++)
        {
            OutputDebugStringW((*it).c_str());
            OutputDebugStringW(L"; ");
        }
        OutputDebugStringW(L"\n");

        this->spine->SetAnimation(this->animeToName[SpineAnime::IDLE].c_str()); // init animation
        this->texture = this->spine->GetTexture();
        this->texWidth = this->texture->GetWidth();
        this->texHeight = this->texture->GetHeight();
        this->textureBrush = new Gdiplus::TextureBrush(this->texture); // create once
        return TRUE;
    }

    this->spine->DisposeResources();
    delete spine;
    this->spine = NULL;
    return FALSE;
}

void SpineChar::UnloadSpine()
{
    if (this->textureBrush)
    {
        delete this->textureBrush;
        this->textureBrush = NULL;
    }
    this->texWidth = 0;
    this->texHeight = 0;
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
        this->Lock(); // 上锁

        // 真实逻辑帧间隔
        elapse = this->timer.GetMilliseconds();

        // 重置计时器
        this->timer.Start();

        // 按真实间隔更新并渲染
        this->Update((UINT)(elapse + 0.5));
        this->Render();

        // 获得这一帧的真实间隔
        elapse = this->timer.GetMilliseconds();
        if (elapse < this->frameInterval)
        {
            // 限制最大帧率
            HighResolutionSleep((DWORD)(this->frameInterval - elapse));
        }

        this->Unlock(); // 解锁
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
    if (!this->spine)
        return FALSE;

    return this->spine->Update((FLOAT)milliseconds / 1000.0f);
}

BOOL SpineChar::Render()
{
    if (!this->spine)
        return FALSE;

    this->graphics->Clear(Gdiplus::Color::Transparent);
    this->vertexBuffer.clear();
    this->vertexIndexBuffer.clear();
    this->spine->GetMeshTriangles(&this->vertexBuffer, &this->vertexIndexBuffer);
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
        case SpineAction::FLUSH:
            this->SetAnime(SpineAnime::IDLE);
            break;
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
        case SpineAction::FLUSH:
            this->SetAnime(SpineAnime::DRAG);
            break;
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
        case SpineAction::FLUSH:
            this->SetAnime(SpineAnime::WORK);
            break;
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
        case SpineAction::FLUSH:
            this->SetAnime(SpineAnime::SLEEP);
            break;
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

const std::list<std::wstring>* SpineChar::GetAnimeNames()
{
    if (!this->spine) 
        return NULL;
    return this->spine->GetAnimeNames();
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

void SpineChar::SetTransparency(UINT transparency)
{
    this->transparency = PercentToAlpha(transparency);
}

UINT SpineChar::GetTransparency() const
{
    return AlphaToPercent(this->transparency);
}

BOOL SpineChar::SetScale(UINT scale)
{
    this->scale = (FLOAT)scale;
    // TODO: 设置缩放
    return TRUE;
}

UINT SpineChar::GetScale() const
{
    return (UINT)this->scale;
}
