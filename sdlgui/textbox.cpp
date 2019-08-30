/*
    sdlgui/textbox.cpp -- Fancy text box with builtin regular
    expression-based validation
    The text box widget was contributed by Christian Schueller.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/window.h>
#include <sdlgui/screen.h>
#include <sdlgui/textbox.h>
#include <sdlgui/theme.h>
#include <sdlgui/entypo.h>
#include <SDL.h>
#include <regex>
#include <iostream>

NAMESPACE_BEGIN(sdlgui)

TextBox::TextBox(Widget *parent,const std::string &value, const std::string& units)
    : Widget(parent),
      mEditable(false),
      mSpinnable(false),
      mCommitted(true),
      mValue(value),
      mDefaultValue(""),
      mAlignment(Alignment::Center),
      mUnits(units),
      mFormat(""),
      mUnitsImage(-1),
      mValidFormat(true),
      mValueTemp(value),
      mCursorPos(-1),
      mSelectionPos(-1),
      mMousePos(Vector2i(-1,-1)),
      mMouseDownPos(Vector2i(-1,-1)),
      mMouseDragPos(Vector2i(-1,-1)),
      mMouseDownModifier(0),
      mTextOffset(0),
      mLastClick(0) 
{
    if (mTheme) 
      mFontSize = mTheme->mTextBoxFontSize;
    _captionTex.dirty = true;
    _unitsTex.dirty = true;
}

void TextBox::setEditable(bool editable) 
{
    mEditable = editable;
    _captionTex.dirty = true;
    setCursor(editable ? Cursor::IBeam : Cursor::Arrow);
}

void TextBox::setTheme(Theme *theme) 
{
    Widget::setTheme(theme);
    if (mTheme)
        mFontSize = mTheme->mTextBoxFontSize;
}

Vector2i TextBox::preferredSize(SDL_Renderer *ctx) const
{
  Vector2i size(0, fontSize() * 1.4f);

    float uw = 0;
    if (mUnitsImage > 0)
    {
      /*  int w, h;
        nvgImageSize(ctx, mUnitsImage, &w, &h);
        float uh = size(1) * 0.4f;
        uw = w * uh / h;
        */
    }
    else if (!mUnits.empty()) 
    {
        uw = const_cast<TextBox*>(this)->mTheme->getUtf8Width("sans", fontSize(), mUnits.c_str());
    }
    float sw = 0;
    if (mSpinnable) 
    {
        sw = 14.f;
    }

    float ts = const_cast<TextBox*>(this)->mTheme->getUtf8Width("sans", fontSize(), mValue.c_str());
    size.x = size.y + ts + uw + sw;
    return size;
}

void TextBox::draw(SDL_Renderer* renderer) 
{
    Widget::draw(renderer);

    SDL_Point ap = getAbsolutePos();

    SDL_Color bgColor = Color(32, 32).toSdlColor();
    SDL_FRect bgRect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1 + 1.0f, mSize.x - 2, mSize.y - 2 };

    SDL_Color fg1Color = Color(150, 32).toSdlColor();
    SDL_FRect fg1Rect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1 + 1.0f, mSize.x - 2, mSize.y - 2 };

    SDL_Color fg2Color = Color(255, 0, 0, 100).toSdlColor();
    SDL_FRect fg2Rect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1 + 1.0f, mSize.x - 2, mSize.y - 2 };

    SDL_Color rColor{};
    if (mEditable && focused())
        rColor = mValidFormat ? fg1Color : fg2Color;
    else if (mSpinnable && mMouseDownPos.x != -1)
        rColor = fg1Color;
    else
        rColor = bgColor;

    SDL_FRect rect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1 + 1.0f, mSize.x - 2, mSize.y - 2 };
    SDL_SetRenderDrawColor(renderer, rColor.r, rColor.g, rColor.b, rColor.a);
    SDL_RenderFillRectF(renderer, &rect);

    SDL_Color bcolor = Color(0, 48).toSdlColor();
    SDL_FRect brect{ getAbsoluteLeft() + 0.5f, getAbsoluteTop() + 0.5f, mSize.x - 1, mSize.y - 1 };
    SDL_SetRenderDrawColor(renderer, bcolor.r, bcolor.g, bcolor.b, bcolor.a);
    SDL_RenderDrawRectF(renderer, &brect);

    Vector2i drawPos = absolutePosition();
    float unitWidth = 0;

    if (mUnitsImage > 0) 
    {
      /*  int w, h;
        nvgImageSize(ctx, mUnitsImage, &w, &h);
        float unitHeight = mSize.y() * 0.4f;
        unitWidth = w * unitHeight / h;
        NVGpaint imgPaint = nvgImagePattern(
            ctx, mPos.x() + mSize.x() - xSpacing - unitWidth,
            drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight, 0,
            mUnitsImage, mEnabled ? 0.7f : 0.35f);
        nvgBeginPath(ctx);
        nvgRect(ctx, mPos.x() + mSize.x() - xSpacing - unitWidth,
                drawPos.y() - unitHeight * 0.5f, unitWidth, unitHeight);
        nvgFillPaint(ctx, imgPaint);
        nvgFill(ctx);
        unitWidth += 2; */
    } 
    else if (!mUnits.empty()) 
    {
      if (_unitsTex.dirty)
        mTheme->getTexAndRectUtf8(renderer, _unitsTex, 0, 0, mUnits.c_str(), "sans", fontSize(), Color(255, mEnabled ? 64 : 32));

      unitWidth = _unitsTex.w()+2;
      SDL_RenderCopy(renderer, _unitsTex, absolutePosition() + Vector2i(mSize.x - unitWidth, (mSize.y - _unitsTex.h()) * 0.5f));
      unitWidth += (2+2);
    }

    float spinArrowsWidth = 0.f;

    /*if (mSpinnable && !focused()) 
    {
        spinArrowsWidth = 14.f;

        nvgFontFace(ctx, "icons");
        nvgFontSize(ctx, ((mFontSize < 0) ? mTheme->mButtonFontSize : mFontSize) * 1.2f);

        bool spinning = mMouseDownPos.x() != -1;
        {
            bool hover = mMouseFocus && spinArea(mMousePos) == SpinArea::Top;
            nvgFillColor(ctx, (mEnabled && (hover || spinning)) ? mTheme->mTextColor : mTheme->mDisabledTextColor);
            auto icon = utf8(ENTYPO_ICON_CHEVRON_UP);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            Vector2f iconPos(mPos.x() + 4.f,
                             mPos.y() + mSize.y()/2.f - xSpacing/2.f);
            nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
        }
    
        {
            bool hover = mMouseFocus && spinArea(mMousePos) == SpinArea::Bottom;
            nvgFillColor(ctx, (mEnabled && (hover || spinning)) ? mTheme->mTextColor : mTheme->mDisabledTextColor);
            auto icon = utf8(ENTYPO_ICON_CHEVRON_DOWN);
            nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            Vector2f iconPos(mPos.x() + 4.f,
                             mPos.y() + mSize.y()/2.f + xSpacing/2.f + 1.5f);
            nvgText(ctx, iconPos.x(), iconPos.y(), icon.data(), nullptr);
        }

        nvgFontSize(ctx, fontSize());
        nvgFontFace(ctx, "sans");
    }
    */

    float xSpacing = 3.f;
    switch (mAlignment) 
    {
        case Alignment::Left:
            drawPos.x = getAbsoluteLeft() + xSpacing + spinArrowsWidth;
            break;
        case Alignment::Right:
            drawPos.x = getAbsoluteLeft() + mSize.x - _captionTex.w() - unitWidth - xSpacing;
            break;
        case Alignment::Center:
            if (mUnits.empty())
              drawPos.x = getAbsoluteLeft() + mSize.x * 0.5f;
            else
              drawPos.x = getAbsoluteLeft() + mSize.x * 0.3f;
            break;
    }

    // clip visible text area
    float clipX = _pos.x + spinArrowsWidth - 1.0f;
    float clipY = _pos.y + 1.0f;
    float clipWidth = mSize.x - unitWidth - spinArrowsWidth + 2.0f;
    float clipHeight = mSize.y - 3.0f;

    Vector2i oldDrawPos(drawPos);
    drawPos.x += mTextOffset; 
    drawPos.y += (mSize.y - _captionTex.h()) / 2;

    if (_captionTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mValue.c_str(), "sans", fontSize(), mEnabled ? mTheme->mTextColor : mTheme->mDisabledTextColor);

    if (mCommitted) 
    {
      SDL_RenderCopy(renderer, _captionTex, drawPos);
    } 
    else 
    {
      int w, h;
      mTheme->getUtf8Bounds("sans", fontSize(), mValueTemp.c_str(), &w, &h);
      float textBound[4] = {drawPos.x, drawPos.y, drawPos.x + w, drawPos.y + h};
      float lineh = textBound[3] - textBound[1];

        // find cursor positions
        updateCursor(textBound[2], mValueTemp);

        // compute text offset
        int prevCPos = mCursorPos > 0 ? mCursorPos - 1 : 0;
        int nextCPos = mCursorPos < mValueTemp.size() ? mCursorPos + 1 : mValueTemp.size();
        float prevCX = cursorIndex2Position(prevCPos, textBound[2], mValueTemp);
        float nextCX = cursorIndex2Position(nextCPos, textBound[2], mValueTemp);

        if (nextCX > clipX + clipWidth)
            mTextOffset -= nextCX - (clipX + clipWidth) + 1;
        if (prevCX < clipX)
            mTextOffset += clipX - prevCX + 1;

        //drawPos.x() = oldDrawPos.x() + mTextOffset;

        if (_tempTex.dirty)
          mTheme->getTexAndRectUtf8(renderer, _tempTex, 0, 0, mValueTemp.c_str(), "sans", fontSize(), mTheme->mTextColor);
       
        // draw text with offset
        SDL_RenderCopy(renderer, _tempTex, oldDrawPos);

        if (mCursorPos > -1) 
        {
            if (mSelectionPos > -1) 
            {
              float caretx = cursorIndex2Position(mCursorPos, textBound[2], mValueTemp);
              float selx = cursorIndex2Position(mSelectionPos, textBound[2], mValueTemp);

                if (caretx > selx)
                    std::swap(caretx, selx);

                // draw selection
                SDL_Color c = Color(255, 255, 255, 80).toSdlColor();
                SDL_FRect sr{ oldDrawPos.x + caretx, oldDrawPos.y + 4, selx - caretx, height() - 4 };
                SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
                SDL_RenderFillRectF(renderer, &sr);
            }

            caretLastTickCount = SDL_GetTicks();
            // draw cursor
            if (caretLastTickCount % 1000 < 500)
            {
              float caretx = cursorIndex2Position(mCursorPos, textBound[2], mValueTemp);

              SDL_Color c = Color(255, 192, 0, 255).toSdlColor();
              SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
              SDL_RenderDrawLineF(renderer, oldDrawPos.x + caretx, oldDrawPos.y + 4,
                oldDrawPos.x + caretx, oldDrawPos.y + lineh - 3);
            }
        }
    }
}

bool TextBox::mouseButtonEvent(const Vector2i &p, int button, bool down,
                               int modifiers)
{
    if (button == SDL_BUTTON_LEFT && down && !mFocused)
    {
        if (!mSpinnable || spinArea(p) == SpinArea::None) /* not on scrolling arrows */
            requestFocus();
    }

    if (mEditable && focused()) 
    {
        if (down) 
        {
            mMouseDownPos = p;
            mMouseDownModifier = modifiers;

            double time = SDL_GetTicks();
            if (time - mLastClick < 0.25) {
                /* Double-click: select all text */
                mSelectionPos = 0;
                mCursorPos = (int) mValueTemp.size();
                mMouseDownPos = Vector2i{ -1, -1 };
            }
            mLastClick = time;
        } 
        else
        {
          mMouseDownPos = Vector2i{ -1, -1 };
          mMouseDragPos = Vector2i{ -1, -1 };
        }
        return true;
    } 
    else if (mSpinnable && !focused()) 
    {
        if (down) 
        {
            if (spinArea(p) == SpinArea::None)
            {
                mMouseDownPos = p;
                mMouseDownModifier = modifiers;

                double time = SDL_GetTicks();
                if (time - mLastClick < 0.25) 
                {
                    /* Double-click: reset to default value */
                    mValue = mDefaultValue;
                    if (mCallback)
                        mCallback(mValue);

                    mMouseDownPos = Vector2i{ -1, -1 };
                }
                mLastClick = time;
            } 
            else 
            {
              mMouseDownPos = Vector2i{ -1, -1 };
              mMouseDragPos = Vector2i{ -1, -1 };
            }
        } 
        else 
        {
            mMouseDownPos = Vector2i{ -1, -1 };
            mMouseDragPos = Vector2i{ -1, -1 };
        }
        return true;
    }

    return false;
}

bool TextBox::mouseMotionEvent(const Vector2i &p, const Vector2i & /* rel */,
                               int /* button */, int /* modifiers */) 
{
    mMousePos = p;

    if (!mEditable)
        setCursor(Cursor::Arrow);
    else if (mSpinnable && !focused() && spinArea(mMousePos) != SpinArea::None) /* scrolling arrows */
        setCursor(Cursor::Hand);
    else
        setCursor(Cursor::IBeam);

    if (mEditable && focused()) 
    {
        return true;
    }
    return false;
}

bool TextBox::mouseDragEvent(const Vector2i &p, const Vector2i &/* rel */,
                             int /* button */, int /* modifiers */) 
{
    mMousePos = p;
    mMouseDragPos = p;

    if (mEditable && focused()) 
    {
        return true;
    }
    return false;
}

bool TextBox::focusEvent(bool focused) 
{
    Widget::focusEvent(focused);

    std::string backup = mValue;

    if (mEditable) 
    {
        if (focused) 
        {
            mValueTemp = mValue;
            _tempTex.dirty = true;
            mCommitted = false;
            mCursorPos = 0;
        } 
        else 
        {
            if (mValidFormat) 
            {
                if (mValueTemp == "")
                    mValue = mDefaultValue;
                else
                    mValue = mValueTemp;
            }

            if (mCallback && !mCallback(mValue))
                mValue = backup;

            mValidFormat = true;
            _captionTex.dirty = true;
            mCommitted = true;
            mCursorPos = -1;
            mSelectionPos = -1;
            mTextOffset = 0;
        }

        mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
    }

    return true;
}

bool TextBox::keyboardEvent(int key, int /* scancode */, int action, int modifiers) 
{
    if (mEditable && focused()) 
    {
        if (action == SDL_PRESSED) 
        {
            if (key == SDLK_LEFT) 
            {
                if (modifiers & KMOD_SHIFT) 
                {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                }
                else 
                {
                    mSelectionPos = -1;
                }

                if (mCursorPos > 0)
                    mCursorPos--;
            } 
            else if (key == SDLK_RIGHT) 
            {
                if (modifiers & KMOD_SHIFT) 
                {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } 
                else 
                {
                    mSelectionPos = -1;
                }

                if (mCursorPos < (int) mValueTemp.length())
                    mCursorPos++;
            } 
            else if (key == SDLK_HOME) 
            {
                if (modifiers & KMOD_SHIFT) 
                {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                } 
                else 
                {
                    mSelectionPos = -1;
                }

                mCursorPos = 0;
            } 
            else if (key == SDLK_END) 
            {
                if (modifiers & KMOD_SHIFT) 
                {
                    if (mSelectionPos == -1)
                        mSelectionPos = mCursorPos;
                }
                else 
                {
                    mSelectionPos = -1;
                }

                mCursorPos = (int) mValueTemp.size();
            } 
            else if (key == SDLK_BACKSPACE) 
            {
                if (!deleteSelection()) 
                {
                    if (mCursorPos > 0) 
                    {
                        mValueTemp.erase(mValueTemp.begin() + mCursorPos - 1);
                        _tempTex.dirty = true;
                        mCursorPos--;
                    }
                }
            } 
            else if (key == SDLK_DELETE) 
            {
                if (!deleteSelection()) 
                {
                    if (mCursorPos < (int) mValueTemp.length())
                        mValueTemp.erase(mValueTemp.begin() + mCursorPos);
                    _tempTex.dirty = true;
                }
            }
            else if (key == SDLK_RETURN) 
            {
                if (!mCommitted)
                    focusEvent(false);
            } 
            else if (key == SDLK_a && modifiers & SDLK_LCTRL) 
            {
                mCursorPos = (int) mValueTemp.length();
                mSelectionPos = 0;
            } 
            else if (key == SDLK_x && modifiers & SDLK_LCTRL)
            {
                copySelection();
                deleteSelection();
            } 
            else if (key == SDLK_c && modifiers & SDLK_LCTRL)
            {
                copySelection();
            } 
            else if (key == SDLK_v && modifiers & SDLK_LCTRL)
            {
                deleteSelection();
                pasteFromClipboard();
            }

            mValidFormat =
                (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
        }
        return true;
    }

    return false;
}

bool TextBox::keyboardCharacterEvent(unsigned int codepoint) 
{
    if (mEditable && focused()) 
    {
        std::ostringstream convert;
        convert << (char) codepoint;

        deleteSelection();
        mValueTemp.insert(mCursorPos, convert.str());
        mCursorPos++;

        mValidFormat = (mValueTemp == "") || checkFormat(mValueTemp, mFormat);
        _tempTex.dirty = true;

        return true;
    }

    return false;
}

bool TextBox::checkFormat(const std::string &input, const std::string &format) 
{
    if (format.empty())
        return true;
    try 
    {
        std::regex regex(format);
        return regex_match(input, regex);
    } 
    catch (const std::regex_error &) 
    {
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
        std::cerr << "Warning: cannot validate text field due to lacking regular expression support. please compile with GCC >= 4.9" << std::endl;
        return true;
#else
        throw;
#endif
    }
}

bool TextBox::copySelection() 
{
    if (mSelectionPos > -1) 
    {
        Screen *sc = dynamic_cast<Screen *>(this->window()->parent());

        int begin = mCursorPos;
        int end = mSelectionPos;

        if (begin > end)
            std::swap(begin, end);

        SDL_SetClipboardText(mValueTemp.substr(begin, end).c_str());
        return true;
    }

    return false;
}

void TextBox::pasteFromClipboard() 
{
    Screen *sc = dynamic_cast<Screen *>(this->window()->parent());
    const char* cbstr = SDL_GetClipboardText();
    if (cbstr)
    {
      mValueTemp.insert(mCursorPos, std::string(cbstr));
      _captionTex.dirty = true;
    }
}

bool TextBox::deleteSelection() 
{
    if (mSelectionPos > -1) 
    {
        int begin = mCursorPos;
        int end = mSelectionPos;

        if (begin > end)
            std::swap(begin, end);

        if (begin == end - 1)
            mValueTemp.erase(mValueTemp.begin() + begin);
        else
            mValueTemp.erase(mValueTemp.begin() + begin,
                             mValueTemp.begin() + end);

        mCursorPos = begin;
        mSelectionPos = -1;
        _tempTex.dirty = true;

        return true;
    }

    return false;
}

void TextBox::updateCursor(float lastx, const std::string& str) 
{
    // handle mouse cursor events
    if (mMouseDownPos.x != -1) 
    {
        if (mMouseDownModifier == KMOD_SHIFT) 
        {
            if (mSelectionPos == -1)
                mSelectionPos = mCursorPos;
        } 
        else
            mSelectionPos = -1;

        mCursorPos = position2CursorIndex(mMouseDownPos.x, lastx, str);

        mMouseDownPos = Vector2i{ -1, -1 };
    } 
    else if (mMouseDragPos.x != -1) 
    {
        if (mSelectionPos == -1)
            mSelectionPos = mCursorPos;

        mCursorPos = position2CursorIndex(mMouseDragPos.x, lastx, str);
    } 
    else 
    {
        // set cursor to last character
        if (mCursorPos == -2)
            mCursorPos = str.size();
    }

    if (mCursorPos == mSelectionPos)
        mSelectionPos = -1;
}

float TextBox::cursorIndex2Position(int index, float lastx, const std::string& str) 
{
    float pos = 0;
    if (index >= str.size())
        pos = _tempTex.w(); // last character
    else
        pos = mTheme->getUtf8Width("sans", fontSize(), str.substr(0, index).c_str());;

    return pos;
}

int TextBox::position2CursorIndex(float posx, float lastx, const std::string& str) 
{
    int mCursorId = 0;
    float caretx = mTheme->getUtf8Width("sans", fontSize(), str.substr(0, mCursorId).c_str());
    for (int j = 1; j <= str.size(); j++) 
    {
      int glposx = mTheme->getUtf8Width("sans", fontSize(), str.substr(0, j).c_str());
        if (std::abs(caretx - posx) > std::abs(glposx - posx)) 
        {
            mCursorId = j;
            caretx = mTheme->getUtf8Width("sans", fontSize(), str.substr(0, mCursorId).c_str()); 
        }
    }
    if (std::abs(caretx - posx) > std::abs(lastx - posx))
        mCursorId = str.size();

    return mCursorId;
}

TextBox::SpinArea TextBox::spinArea(const Vector2i & pos)
{
    if (0 <= pos.x - _pos.x && pos.x - _pos.x < 14.f) 
    { /* on scrolling arrows */
        if (mSize.y >= pos.y - _pos.y && pos.y - _pos.y <= mSize.y / 2.f) 
        { /* top part */
            return SpinArea::Top;
        } 
        else if (0.f <= pos.y - _pos.y && pos.y - _pos.y > mSize.y / 2.f) 
        { /* bottom part */
            return SpinArea::Bottom;
        }
    }
    return SpinArea::None;
}

NAMESPACE_END(sdlgui)

