/*
    sdlgui/button.cpp -- [Normal/Toggle/Radio/Popup] Button widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/button.h>
#include <sdlgui/theme.h>
#include <SDL.h>
#include <array>

NAMESPACE_BEGIN(sdlgui)

Button::Button(Widget *parent, const std::string &caption, int icon)
    : Widget(parent), mCaption(caption), mIcon(icon),
      mIconPosition(IconPosition::LeftCentered), mPushed(false),
      mFlags(NormalButton), mBackgroundColor(Color(0, 0)),
      mTextColor(Color(0, 0)) 
{
  _captionTex.dirty = true;
  _iconTex.dirty = true;
}

Vector2i Button::preferredSize(SDL_Renderer *ctx) const
{
    int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
    float tw = const_cast<Button*>(this)->mTheme->getTextWidth("sans-bold", fontSize, mCaption.c_str());
    float iw = 0.0f, ih = fontSize;

    if (mIcon) 
    {
        if (nvgIsFontIcon(mIcon)) 
        {
            ih *= 1.5f;
            iw = const_cast<Button*>(this)->mTheme->getUtf8Width("icons", ih, utf8(mIcon).data())  + mSize.y * 0.15f;
        } 
        else 
        {
            int w, h;
            ih *= 0.9f;
            SDL_QueryTexture((SDL_Texture*)mIcon, nullptr, nullptr, &w, &h);
            iw = w * ih / h;
        }
    }
    return Vector2i((int)(tw + iw) + 20, fontSize + 10);
}

bool Button::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    Widget::mouseButtonEvent(p, button, down, modifiers);
    /* Temporarily increase the reference count of the button in case the
       button causes the parent window to be destructed */
    ref<Button> self = this;

    if (button ==  SDL_BUTTON_LEFT && mEnabled) 
    {
        bool pushedBackup = mPushed;
        if (down) 
        {
            if (mFlags & RadioButton) 
            {
                if (mButtonGroup.empty()) 
                {
                    for (auto widget : parent()->children()) 
                    {
                        Button *b = dynamic_cast<Button *>(widget);
                        if (b != this && b && (b->flags() & RadioButton) && b->mPushed) 
                        {
                            b->mPushed = false;
                            if (b->mChangeCallback)
                                b->mChangeCallback(false);
                        }
                    }
                } 
                else 
                {
                    for (auto b : mButtonGroup) 
                    {
                        if (b != this && (b->flags() & RadioButton) && b->mPushed) 
                        {
                            b->mPushed = false;
                            if (b->mChangeCallback)
                                b->mChangeCallback(false);
                        }
                    }
                }
            }

            if (mFlags & PopupButton) 
            {
                for (auto widget : parent()->children()) 
                {
                    Button *b = dynamic_cast<Button *>(widget);
                    if (b != this && b && (b->flags() & PopupButton) && b->mPushed) 
                    {
                        b->mPushed = false;
                        if (b->mChangeCallback)
                            b->mChangeCallback(false);
                    }
                }
            }

            if (mFlags & ToggleButton)
                mPushed = !mPushed;
            else
                mPushed = true;
        } 
        else if (mPushed) 
        {
            if (contains(p) && mCallback)
                mCallback();
            if (mFlags & NormalButton)
                mPushed = false;
        }
        if (pushedBackup != mPushed && mChangeCallback)
            mChangeCallback(mPushed);

        if (pushedBackup != mPushed)
        {
          _captionTex.dirty = true;
          _iconTex.dirty = true;
        }
        return true;
    }
    return false;
}

void Button::setTextColor(const Color &textColor) 
{ 
  mTextColor = textColor; 
  _captionTex.dirty = true;
  _iconTex.dirty = true;
}

void Button::draw(SDL_Renderer* renderer)
{
  Widget::draw(renderer);

  SDL_Color bodyColor = mTheme->mButtonGradientTopUnfocused.toSdlColor();
  if (mBackgroundColor.a() != 0)
    bodyColor = mBackgroundColor.toSdlColor();

  if (mPushed) 
  {
    if (mBackgroundColor.a() != 0)
    {
      bodyColor.b *= 1.5;
      bodyColor.g *= 1.5;
      bodyColor.r *= 1.5;
    }
    else
      bodyColor = mTheme->mButtonGradientTopPushed.toSdlColor();
  }
  else if (mMouseFocus && mEnabled)
  {
    if (mBackgroundColor.a() != 0)
    {
      bodyColor.b *= 0.5;
      bodyColor.g *= 0.5;
      bodyColor.r *= 0.5;
    }
    else
      bodyColor = mTheme->mButtonGradientTopFocused.toSdlColor();
  }

  SDL_FRect bodyRect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1.0f, width() - 2, height() - 2 };
  SDL_SetRenderDrawColor(renderer, bodyColor.r, bodyColor.g, bodyColor.b, bodyColor.a);
  SDL_RenderFillRectF(renderer, &bodyRect);

  SDL_FRect btnRect{ getAbsoluteLeft() - 1.5f, getAbsoluteTop() - 1.0f, width() + 3.f, height() + 1 };


  SDL_Color bl = (mPushed ? mTheme->mBorderDark : mTheme->mBorderLight).toSdlColor();
  SDL_SetRenderDrawColor(renderer, bl.r, bl.g, bl.b, bl.a);
  SDL_FRect blr{ getAbsoluteLeft(), getAbsoluteTop() + (mPushed ? 0.5f : 1.5f), width() - 1,
                 height() - 1 - (mPushed ? 0.0f : 1.0f) };
  SDL_RenderDrawLineF(renderer, blr.x, blr.y, blr.x + blr.w, blr.y);
  SDL_RenderDrawLineF(renderer, blr.x, blr.y, blr.x, blr.y + blr.h - 1);

  SDL_Color bd = (mPushed ? mTheme->mBorderLight : mTheme->mBorderDark).toSdlColor();
  SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
  SDL_FRect bdr{ getAbsoluteLeft(), getAbsoluteTop() + 0.5f, width() - 1, height() - 2 };
  SDL_RenderDrawLineF(renderer, bdr.x, bdr.y + bdr.h, bdr.x + bdr.w, bdr.y + bdr.h);
  SDL_RenderDrawLineF(renderer, bdr.x + bdr.w, bdr.y, bdr.x + bdr.w, bdr.y + bdr.h);

  bd = mTheme->mBorderDark.toSdlColor();
  SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
  SDL_RenderDrawRectF(renderer, &btnRect);

  int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
  if (_captionTex.dirty)
  {
    Color sdlTextColor = (mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor);
    if (!mEnabled)
      sdlTextColor = mTheme->mDisabledTextColor;

    mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mCaption.c_str(), "sans-bold", fontSize, sdlTextColor);
  }

  Vector2f center(getAbsoluteLeft() + width() * 0.5f, getAbsoluteTop() + height() * 0.5f);
  Vector2i textPos(center.x - _captionTex.w() * 0.5f, center.y - _captionTex.h() * 0.5f - 1);
  
  int offset = mPushed ? 2 : 0;

  if (mIcon) 
  {
    float iw = 0, ih = fontSize;
    auto icon = utf8(mIcon);

    if (_iconTex.dirty)
    {
      Color sdlTextColor = (mTextColor.a() == 0 ? mTheme->mTextColor : mTextColor);

      if (nvgIsFontIcon(mIcon))
      {
        ih *= 1.5f;
        mTheme->getTexAndRectUtf8(renderer, _iconTex, 0, 0, icon.data(), "icons", ih, sdlTextColor);
        iw = _iconTex.w();
      }
      else
      {
        int w, h;
        ih *= 0.9f;
        iw = _iconTex.w() * ih / _iconTex.h();
      }

    }
    if (mCaption != "")
      iw += _pos.y * 0.15f;

    Vector2i iconPos = center.As<int>();
    iconPos.y -= 1;

    if (mIconPosition == IconPosition::LeftCentered) 
    {
      iconPos.x -= _captionTex.w() * 0.5f;
      iconPos.x -= _iconTex.w() * 0.5f;
      textPos.x += _iconTex.w() * 0.5f;// iw * 0.5f;
    }
    else if (mIconPosition == IconPosition::RightCentered) 
    {
      textPos.x -= iw * 0.5f;
      iconPos.x += _captionTex.w() * 0.5f;
    }
    else if (mIconPosition == IconPosition::Left) 
    {
      iconPos.x = getAbsoluteLeft() + 8;
    }
    else if (mIconPosition == IconPosition::Right) 
    {
      iconPos.x = getAbsoluteLeft() + width() - iw - 8;
    }

    if (nvgIsFontIcon(mIcon)) 
      SDL_RenderCopy(renderer, _iconTex, iconPos + Vector2i(offset, offset - _iconTex.h() * 0.5f + 1));
    else 
      SDL_RenderCopy(renderer, _iconTex, iconPos + Vector2i(offset, - ih / 2 + offset));
  }

  SDL_RenderCopy(renderer, _captionTex, textPos + Vector2i(offset, 1 + offset));
}

NAMESPACE_END(sdlgui)
