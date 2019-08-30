/*
    sdlgui/progressbar.cpp -- Standard widget for visualizing progress

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/progressbar.h>
#include <sdlgui/theme.h>

NAMESPACE_BEGIN(sdlgui)

ProgressBar::ProgressBar(Widget *parent)
    : Widget(parent), mValue(0.0f) 
{
}

Vector2i ProgressBar::preferredSize(SDL_Renderer *) const
{
    return Vector2i(70, 12);
}

void ProgressBar::draw(SDL_Renderer* renderer)
{
  Widget::draw(renderer);

  SDL_Color bd = mTheme->mBorderDark.toSdlColor();
  SDL_FRect bdRect{ getAbsoluteLeft() - 1, getAbsoluteTop() - 1, mSize.x + 2, mSize.y + 2 };
  SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
  SDL_RenderDrawRectF(renderer, &bdRect);

  SDL_Color bg = mTheme->mBorderMedium.toSdlColor();
  SDL_FRect bgRect { getAbsoluteLeft(), getAbsoluteTop(), mSize.x, mSize.y };
  SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRectF(renderer, &bgRect);

  float value = std::min(std::max(0.0f, mValue), 1.0f);
  if (value > 0)
  {
    int barPos = (int)std::round((mSize.x - 2) * value);

    SDL_Color br = mTheme->mBorderLight.toSdlColor();
    SDL_FRect brRect{ getAbsoluteLeft() + 1, getAbsoluteTop() + 1, barPos - 1, mSize.y - 2 };
    SDL_SetRenderDrawColor(renderer, br.r, br.g, br.b, br.a);
    SDL_RenderFillRectF(renderer, &brRect);
  }
}

NAMESPACE_END(sdlgui)
