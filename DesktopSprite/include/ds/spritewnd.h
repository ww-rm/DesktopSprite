#pragma once
#ifndef DS_SPRITEWND_H
#define DS_SPRITEWND_H

#include <ds/framework.h>
#include <ds/spinechar.h>
#include <ds/config.h>

#include <ds/basewindow.h>

using namespace Gdiplus;


class SpriteWindow : public BaseWindow
{
private:
    SIZE sysDragSize = { 0 };               // 系统拖动判定大小
    POINT ptDragSrc = { 0 };                // 拖动窗口时的源点
    POINT ptSpriteDragSrc = { 0 };          // 拖动窗口时精灵的位置
    BOOL isDragging = FALSE;
    SpineChar* spinechar = NULL;
    SpineRenderer* spinerenderer = NULL;

public:
    SpriteWindow();
    PCWSTR GetClassName_() const;
    SpineChar* GetSpineChar();

    BOOL LoadLastPosFromReg(POINT* pt);
    BOOL SaveCurrentPosToReg();

    BOOL LoadFlipXFromReg(BOOL* flip);
    BOOL SaveFlipXToReg();

    BOOL ApplyConfig(const AppConfig::AppConfig* newConfig = NULL);

private:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnCreate(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);
    LRESULT OnClose(WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(WPARAM wParam, LPARAM lParam);
    LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
    LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
    LRESULT OnInitMenuPopup(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
    LRESULT OnLButtonDBClick(WPARAM wParam, LPARAM lParam);
    LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
};


#endif // !DS_SPRITEWND_H
