/*
    src/vgbutton.cpp -- [Normal/Toggle/Radio/Popup] Button widget
    with nanovg rasterizer

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>
    
    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/vgbutton.h>
#include <sdlgui/theme.h>
#include <thread>
#include <iostream>
#include <mutex>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION
#include "nanovg_rt.h"


NAMESPACE_BEGIN(sdlgui)

struct vgButton::AsyncTexture
{
  int id;
  static std::mutex i_mutex;
	Texture tex;
  NVGcontext* ctx = nullptr;

  AsyncTexture(int _id) : id(_id) {};

	void load(vgButton* ptr, bool pushed, bool focused, bool enabled)
	{
    vgButton* button = ptr;
    AsyncTexture* self = this;
    std::thread tgr([=]() {
      Theme* theme = button->theme();
      Color backgroundColor = button->backgroundColor();
      std::lock_guard<std::mutex> guard(i_mutex);

      int ww = button->width();
      int hh = button->height();
      NVGcontext *ctx = nvgCreateRT(NVG_DEBUG, ww+2, hh+2);

      float pxRatio = 1.0f;
      nvgClearBackgroundRT(ctx, 0, 0, 0, 0.0f);
      nvgBeginFrame(ctx, ww+2, hh+2, pxRatio);

      NVGcolor gradTop = theme->mButtonGradientTopUnfocused.toNvgColor();
      NVGcolor gradBot = theme->mButtonGradientBotUnfocused.toNvgColor();

      if (pushed)
      {
        gradTop = theme->mButtonGradientTopPushed.toNvgColor();
        gradBot = theme->mButtonGradientBotPushed.toNvgColor();
      }
      else if (focused && enabled)
      {
        gradTop = theme->mButtonGradientTopFocused.toNvgColor();
        gradBot = theme->mButtonGradientBotFocused.toNvgColor();
      }

      nvgBeginPath(ctx);

      nvgRoundedRect(ctx, 1, 1.0f, ww - 2, hh - 2, theme->mButtonCornerRadius - 1);

      if (backgroundColor.a() != 0)
      {
        nvgFillColor(ctx, backgroundColor.toNvgColor());
        nvgFill(ctx);
        if (pushed)
        {
          gradTop.a = gradBot.a = 0.8f;
        }
        else
        {
          double v = 1 - backgroundColor.a();
          gradTop.a = gradBot.a = enabled ? v : v * .5f + .5f;
        }
      }

      NVGpaint bg = nvgLinearGradient(ctx, 0, 0, 0, hh, gradTop, gradBot);

      nvgFillPaint(ctx, bg);
      nvgFill(ctx);

      nvgBeginPath(ctx);
      nvgStrokeWidth(ctx, 1.0f);
      nvgRoundedRect(ctx, 0.5f, (pushed ? 0.5f : 1.5f), ww - 1, hh - 1 - (pushed ? 0.0f : 1.0f), theme->mButtonCornerRadius);
      nvgStrokeColor(ctx, theme->mBorderLight.toNvgColor());
      nvgStroke(ctx);

      nvgBeginPath(ctx);
      nvgRoundedRect(ctx, 0.5f, 0.5f, ww - 1, hh - 2, theme->mButtonCornerRadius);
      nvgStrokeColor(ctx, theme->mBorderDark.toNvgColor());
      nvgStroke(ctx);

      nvgEndFrame(ctx);
      self->tex.rrect = { 0, 0, ww+2, hh+2 };
      self->ctx = ctx;
    });

    tgr.detach();
  }

  void perform(SDL_Renderer* renderer)
  {
    if (!ctx)
      return;

    unsigned char *rgba = nvgReadPixelsRT(ctx);

    tex.tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, tex.w(), tex.h());

    int pitch;
    uint8_t *pixels;
    int ok = SDL_LockTexture(tex.tex, nullptr, (void **)&pixels, &pitch);
    memcpy(pixels, rgba, sizeof(uint32_t) * tex.w() * tex.h());
    SDL_SetTextureBlendMode(tex.tex, SDL_BLENDMODE_BLEND);
    SDL_UnlockTexture(tex.tex);

    nvgDeleteRT(ctx);
    ctx = nullptr;
  }
};

std::mutex vgButton::AsyncTexture::i_mutex;

vgButton::vgButton(Widget *parent, const std::string &caption, int icon)
	: Button(parent, caption, icon) 
{

}

void vgButton::drawBody(SDL_Renderer* renderer)
{
  int id = (mPushed?0x1:0) + (mMouseFocus?0x2:0) + (mEnabled?0x4:0);

  AsyncTexturePtr atx;
  for (auto& txid : _txs)
  {
    if (txid->id == id)
    {
      atx = txid;
      break;
    }
  }

  if (atx)
  {
    Vector2i ap = absolutePosition();
    atx->perform(renderer);
    SDL_RenderCopy(renderer, atx->tex, ap);
  }
  else
  {
    AsyncTexturePtr newtx = std::make_shared<AsyncTexture>(id);
    newtx->load(this, mPushed, mMouseFocus, mEnabled);
    _txs.push_back(newtx);
  }
}

NAMESPACE_END(sdlgui)