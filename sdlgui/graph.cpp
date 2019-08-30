/*
    sdlgui/graph.cpp -- Simple graph widget for showing a function plot

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/graph.h>
#include <sdlgui/theme.h>

NAMESPACE_BEGIN(sdlgui)

Graph::Graph(Widget *parent, const std::string &caption)
    : Widget(parent), mCaption(caption) 
{
    mBackgroundColor = Color(20, 128);
    mForegroundColor = Color(255, 192, 0, 128);
    mTextColor = Color(240, 192);
    _captionTex.dirty = true;
    _headerTex.dirty = true;
}

Vector2i Graph::preferredSize(SDL_Renderer *) const
{
    return Vector2i(180, 45);
}

void Graph::draw(SDL_Renderer *renderer) 
{
    Widget::draw(renderer);

    Vector2i ap = absolutePosition();
    
    SDL_Color bg = mBackgroundColor.toSdlColor();
    SDL_FRect bgrect{ ap.x, ap.y, mSize.x, mSize.y };
    
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRectF(renderer, &bgrect);

    if (mValues.size() < 2)
        return;

    SDL_Color fc = mForegroundColor.toSdlColor();
    SDL_SetRenderDrawColor(renderer, fc.r, fc.g, fc.b, fc.a);

    for (size_t i = 0; i < (size_t) mValues.size(); i++) 
    {
        float value = mValues[i];
        float vx = ap.x + i * mSize.x / (float) (mValues.size() - 1);
        float vy = ap.y + (1-value) * mSize.y;
        SDL_RenderDrawLineF(renderer, vx, ap.y + height(), vx, vy);
    }

    if (_captionTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _captionTex, 0, 0, mCaption.c_str(), "sans", 14, mTextColor);

    if (_headerTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _headerTex, 0, 0, mHeader.c_str(), "sans", 18, mTextColor);

    if (_footerTex.dirty)
      mTheme->getTexAndRectUtf8(renderer, _footerTex, 0, 0, mFooter.c_str(), "sans", 15, mTextColor);

    SDL_RenderCopy(renderer, _captionTex, ap + Vector2i(3,1) );
    SDL_RenderCopy(renderer, _headerTex, ap + Vector2i(mSize.x - 3 - _headerTex.w(), 1));
    SDL_RenderCopy(renderer, _footerTex, ap + Vector2i(mSize.x - 3 - _footerTex.w(), mSize.y - 1 - _footerTex.h()));
 }

NAMESPACE_END(sdlgui)
