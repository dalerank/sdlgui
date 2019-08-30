/*
    sdl_gui/progressbar.h -- Standard widget for visualizing progress

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
 * \class ProgressBar progressbar.h sdl_gui/progressbar.h
 *
 * \brief Standard widget for visualizing progress.
 */
class  ProgressBar : public Widget 
{
public:
    ProgressBar(Widget *parent);

    float value() { return mValue; }
    void setValue(float value) { mValue = value; }

    Vector2i preferredSize(SDL_Renderer *ctx) const override;
    void draw(SDL_Renderer* renderer) override;

protected:
    float mValue;
};

NAMESPACE_END(sdlgui)
