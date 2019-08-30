/*
    sdlgui/slider.cpp -- Fractional slider widget with mouse control

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/slider.h>
#include <sdlgui/theme.h>
#include <sdlgui/entypo.h>
#include <array>

NAMESPACE_BEGIN(sdlgui)

Slider::Slider(Widget *parent, float value)
    : Widget(parent), mValue(value), mHighlightedRange(std::make_pair(0.f, 0.f)) 
{
    mHighlightColor = Color(255, 80, 80, 70);
    _innerKnobTex.dirty = true;
}

Vector2i Slider::preferredSize(SDL_Renderer *) const
{
    return Vector2i(70, 12);
}

bool Slider::mouseDragEvent(const Vector2i &p, const Vector2i & /* rel */,
                            int /* button */, int /* modifiers */) 
{
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x - _pos.x) / (float) mSize.x, (float) 0.0f), (float) 1.0f);
    if (mCallback) mCallback(mValue);
    if (mObjCallback) mObjCallback(this, mValue);
    return true;
}

bool Slider::mouseButtonEvent(const Vector2i &p, int /* button */, bool down, int /* modifiers */)
{
    if (!mEnabled)
        return false;
    mValue = std::min(std::max((p.x - _pos.x) / (float) mSize.x, (float) 0.0f), (float) 1.0f);
    if (mCallback) mCallback(mValue);
    if (mObjCallback) mObjCallback(this, mValue);
    if (mFinalCallback && !down) mFinalCallback(mValue);
    return true;
}

void Slider::draw(SDL_Renderer* renderer) 
{
  Vector2i ap = absolutePosition();
  Vector2f center( ap.x + width() * 0.5f, ap.y + height() * 0.5f);
  Vector2f knobPos( ap.x + mValue * mSize.x, center.y);

  if (_innerKnobTex.dirty)
  {
    mTheme->getTexAndRectUtf8(renderer, _innerKnobTex, 0, 0, utf8(ENTYPO_ICON_RECORD).data(), "icons", 18, mTheme->mSliderKnobInner);
    mTheme->getTexAndRectUtf8(renderer, _outerKnobTex, 0, 0, utf8(ENTYPO_ICON_RECORD).data(), "icons", 34, mTheme->mSliderKnobOuter);
  }

  SDL_Color bg = Color(0, mEnabled ? 128 : 210).toSdlColor();
  SDL_FRect bgRect{ ap.x, center.y - 3 + 1, width(), 6 };

  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRectF(renderer, &bgRect);

  if (mHighlightedRange.second != mHighlightedRange.first) 
  {
    SDL_Color hl = mHighlightColor.toSdlColor();
    SDL_FRect hlRect{ ap.x + mHighlightedRange.first * width(), center.y - 3 + 1, 
                      width() * (mHighlightedRange.second - mHighlightedRange.first), 6 };

    SDL_SetRenderDrawColor(renderer, hl.r, hl.g, hl.b, hl.a);
    SDL_RenderFillRectF(renderer, &hlRect);
  }

  SDL_RenderCopy(renderer, _outerKnobTex, (knobPos + Vector2f( - _outerKnobTex.w() / 2.f, - _outerKnobTex.h() / 2.f)).As<int>());
  SDL_RenderCopy(renderer, _innerKnobTex, (knobPos + Vector2f( - _innerKnobTex.w() / 2.f, - _innerKnobTex.h() / 2.f)).As<int>());
}

NAMESPACE_END(sdlgui)
