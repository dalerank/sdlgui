/*
    sdlgui/checkbox.cpp -- Two-state check box widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/checkbox.h>
#include <SDL.h>
#include <sdlgui/theme.h>
#include <sdlgui/entypo.h>
#include <array>

NAMESPACE_BEGIN(sdlgui)

CheckBox::CheckBox(Widget *parent, const std::string &caption,
                   const std::function<void(bool) > &callback)
    : Widget(parent), mCaption(caption), mPushed(false), mChecked(false),
      mCallback(callback) 
{
  _captionTex.dirty = true;
  _pointTex.dirty = true;
}

bool CheckBox::mouseButtonEvent(const Vector2i &p, int button, bool down,
                                int modifiers) 
{
    Widget::mouseButtonEvent(p, button, down, modifiers);
    if (!mEnabled)
        return false;

    if (button == SDL_BUTTON_LEFT) 
    {
        if (down) 
        {
            mPushed = true;
        } 
        else if (mPushed) 
        {
            if (contains(p)) 
            {
                mChecked = !mChecked;
                if (mCallback)
                    mCallback(mChecked);
            }
            mPushed = false;
        }
        return true;
    }
    return false;
}

Vector2i CheckBox::preferredSize(SDL_Renderer *ctx) const
{
    if (mFixedSize != Vector2i::Zero())
        return mFixedSize;

    int w, h;
    const_cast<CheckBox*>(this)->mTheme->getTextBounds("sans", fontSize(), mCaption.c_str(), &w, &h);
    return Vector2i(w + 1.7f * fontSize(),  fontSize() * 1.3f);
}

void CheckBox::draw(SDL_Renderer *renderer)
{
  Widget::draw(renderer);

  if (_captionTex.dirty)
  {
    Color tColor = (mEnabled ? mTheme->mTextColor : mTheme->mDisabledTextColor);
    mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mCaption.c_str(), "sans", fontSize(), tColor);
    mTheme->getTexAndRectUtf8(renderer, _pointTex, 0, 0, utf8(ENTYPO_ICON_CHECK).data(), "icons", 1.8 * mSize.y, tColor);
  }
 
  auto ap = absolutePosition();
  SDL_RenderCopy(renderer, _captionTex, ap + Vector2i(1.2f * mSize.y + 5, (mSize.y - _captionTex.h()) * 0.5f) );

  SDL_Color bg = (mPushed ? Color(0, 100) : Color(0, 32)).toSdlColor();
  SDL_Color br = mTheme->mBorderDark.toSdlColor();
  SDL_FRect bgrect{ getAbsoluteLeft() + 1.5f, getAbsoluteTop() + 1.5f, mSize.y - 2.0f, mSize.y - 2.0f };
  SDL_SetRenderDrawColor(renderer, br.r, br.g, br.b, br.a);
  SDL_RenderDrawRectF(renderer, &bgrect);

  SDL_FRect mrect{ getAbsoluteLeft() + 1.0f, getAbsoluteTop() + 1.0f, mSize.y - 2.0f,  mSize.y - 2.0f };
  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRectF(renderer, &mrect);
  
  if (mChecked) 
    SDL_RenderCopy(renderer, _pointTex, ap + Vector2i((mSize.y - _pointTex.w()) * 0.5f + 1,  (mSize.y - _pointTex.h()) * 0.5f));
}

NAMESPACE_END(sdlgui)
