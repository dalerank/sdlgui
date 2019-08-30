/*
    sdlgui/window.cpp -- Top-level window widget

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/window.h>
#include <sdlgui/theme.h>
#include <sdlgui/screen.h>
#include <sdlgui/layout.h>
#include <SDL.h>

NAMESPACE_BEGIN(sdlgui)

Window::Window(Widget *parent, const std::string &title)
    : Widget(parent), mTitle(title), mButtonPanel(nullptr), mModal(false), mDrag(false) 
{
  _titleTex.dirty = true;
}

Vector2i Window::preferredSize(SDL_Renderer *ctx) const
{
    if (mButtonPanel)
        mButtonPanel->setVisible(false);
    Vector2i result = Widget::preferredSize(ctx);
    if (mButtonPanel)
        mButtonPanel->setVisible(true);

    int w, h;
    const_cast<Window*>(this)->mTheme->getTextBounds("sans-bold", 18.0, mTitle.c_str(), &w, &h);

    return result.cmax(Vector2i(w + 20, h));
}

Widget *Window::buttonPanel() {
    if (!mButtonPanel) {
        mButtonPanel = new Widget(this);
        mButtonPanel->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 4));
    }
    return mButtonPanel;
}

void Window::performLayout(SDL_Renderer *ctx) 
{
    if (!mButtonPanel) {
        Widget::performLayout(ctx);
    } else {
        mButtonPanel->setVisible(false);
        Widget::performLayout(ctx);
        for (auto w : mButtonPanel->children()) 
        {
          w->setFixedSize({ 22, 22 });
          w->setFontSize(15);
        }
        mButtonPanel->setVisible(true);
        mButtonPanel->setSize({ width(), 22 });
        mButtonPanel->setPosition({ width() - (mButtonPanel->preferredSize(ctx).x + 5), 3 });
        mButtonPanel->performLayout(ctx);
    }
}

bool Window::focusEvent(bool focused)
{
  _titleTex.dirty = focused != mFocused;
  return Widget::focusEvent(focused);
}

void Window::draw(SDL_Renderer* renderer)
{
  int ds = mTheme->mWindowDropShadowSize;
  int cr = mTheme->mWindowCornerRadius;
  int hh = mTheme->mWindowHeaderHeight;

  SDL_Rect rect{ _pos.x, _pos.y, mSize.x, mSize.y };

  /* Draw a drop shadow */
  SDL_Rect shadowRect{ _pos.x - ds,_pos.y - ds, mSize.x + 2 * ds, mSize.y + 2 * ds };
  SDL_Color shadowColor = mTheme->mDropShadow.toSdlColor();

  SDL_SetRenderDrawColor(renderer, shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);
  SDL_RenderFillRect(renderer, &shadowRect);

  /* Draw window */
  SDL_Color color = (mMouseFocus ? mTheme->mWindowFillFocused
                                 : mTheme->mWindowFillUnfocused).toSdlColor();
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderFillRect(renderer, &rect);

  if (_titleTex.dirty)
  {
    Color titleTextColor = (mFocused ? mTheme->mWindowTitleFocused : mTheme->mWindowTitleUnfocused);
    mTheme->getTexAndRectUtf8(renderer, _titleTex, 0, 0, mTitle.c_str(), "sans-bold", 18, titleTextColor);
  }

  SDL_FRect wndBdRect{ getAbsoluteLeft() - 1.5f, getAbsoluteTop() - 1.5f, width() + 3.5f, height() + 3.5f };
  SDL_Color bd = mTheme->mBorderDark.toSdlColor();
  SDL_SetRenderDrawColor(renderer, bd.r, bd.g, bd.b, bd.a);
  SDL_RenderDrawRectF(renderer, &wndBdRect);

  if (!mTitle.empty() && _titleTex.tex) 
  {
    SDL_Color headerColor = mTheme->mWindowHeaderGradientTop.toSdlColor();
    SDL_Rect headerRect{ _pos.x, _pos.y, mSize.x, hh };

    SDL_SetRenderDrawColor(renderer, headerColor.r, headerColor.g, headerColor.b, headerColor.a);
    SDL_RenderFillRect(renderer, &headerRect);

    SDL_Color headerBotColor = mTheme->mWindowHeaderSepBot.toSdlColor();
    SDL_SetRenderDrawColor(renderer, headerBotColor.r, headerBotColor.g, headerBotColor.b, headerBotColor.a);
    SDL_RenderDrawLine(renderer, _pos.x + 0.5f, _pos.y + hh - 1.5f, _pos.x + mSize.x - 0.5f, _pos.y + hh - 1.5);

    SDL_RenderCopy(renderer, _titleTex, _pos + Vector2i( (mSize.x - _titleTex.w())/2, (hh - _titleTex.h()) / 2));
  }

  Widget::draw(renderer);
}

void Window::dispose() 
{
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->disposeWindow(this);
}

void Window::center() 
{
    Widget *widget = this;
    while (widget->parent())
        widget = widget->parent();
    ((Screen *) widget)->centerWindow(this);
}

bool Window::mouseDragEvent(const Vector2i &, const Vector2i &rel,
                            int button, int /* modifiers */) 
{
    if (mDrag && (button & (1 << SDL_BUTTON_LEFT)) != 0) 
    {
        _pos += rel;
        _pos = _pos.cmax({ 0, 0 });
        _pos = _pos.cmin(parent()->size() - mSize);
        return true;
    }
    return false;
}

bool Window::mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers)
{
    if (Widget::mouseButtonEvent(p, button, down, modifiers))
        return true;
    if (button == SDL_BUTTON_LEFT) {
        mDrag = down && (p.y - _pos.y) < mTheme->mWindowHeaderHeight;
        return true;
    }
    return false;
}

bool Window::scrollEvent(const Vector2i &p, const Vector2f &rel)
{
    Widget::scrollEvent(p, rel);
    return true;
}

void Window::refreshRelativePlacement() 
{
    /* Overridden in \ref Popup */
}

NAMESPACE_END(sdlgui)
