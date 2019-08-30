/*
    sdl_gui/slider.h -- Fractional slider widget with mouse control

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/widget.h>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class Slider slider.h sdl_gui/slider.h
 *
 * \brief Fractional slider widget with mouse control.
 */
class  Slider : public Widget 
{
public:
    Slider(Widget *parent, float value = 0.f);

    Slider(Widget *parent, float value, const std::function<void(Slider*, float)>& objcallback)
      : Slider(parent, value) { setObjCallback(objcallback); }

    Slider(Widget *parent, float value, 
           const std::function<void(Slider*, float)>& objcallback,
           const std::function<void(float)> &fcallback)
      : Slider(parent, value) { setObjCallback(objcallback); setFinalCallback(fcallback); }


    float value() const { return mValue; }
    void setValue(float value) { mValue = value; }

    const Color &highlightColor() const { return mHighlightColor; }
    void setHighlightColor(const Color &highlightColor) { mHighlightColor = highlightColor; }

    std::pair<float, float> highlightedRange() const { return mHighlightedRange; }
    void setHighlightedRange(std::pair<float, float> highlightedRange) { mHighlightedRange = highlightedRange; }

    std::function<void(float)> callback() const { return mCallback; }
    void setCallback(const std::function<void(float)> &callback) { mCallback = callback; }

    std::function<void(Slider*,float)> objcallback() const { return mObjCallback; }
    void setObjCallback(const std::function<void(Slider*,float)> &callback) { mObjCallback = callback; }

    std::function<void(float)> finalCallback() const { return mFinalCallback; }
    void setFinalCallback(const std::function<void(float)> &callback) { mFinalCallback = callback; }

    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers) override;
    bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers) override;
    void draw(SDL_Renderer* renderer) override;

    void setKnobOutterRadiusKoeff(float koeff) { mKnobRadKoeff.outter = koeff; }
    void setKnobInnerRadiusKoeff(float koeff) { mKnobRadKoeff.inner = koeff; }

protected:
    float mValue;

    struct 
    {
        float outter=1.f;
        float inner=0.5f;
    } mKnobRadKoeff;

    Texture _innerKnobTex;
    Texture _outerKnobTex;

    std::function<void(float)> mCallback;
    std::function<void(Slider*,float)> mObjCallback;
    std::function<void(float)> mFinalCallback;
    std::pair<float, float> mHighlightedRange;
    Color mHighlightColor;
};

NAMESPACE_END(sdlgui)
