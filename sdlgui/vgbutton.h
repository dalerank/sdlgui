/*
    sdlgui/vgbutton.h -- [Normal/Toggle/Radio/Popup] Button widget with 
    nanovg rasterizer

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <sdlgui/button.h>
#include <vector>
#include <memory>

NAMESPACE_BEGIN(sdlgui)

/**
 * \class vgButton button.h sdlgui/vgbutton.h
 *
 * \brief [Normal/Toggle/Radio/Popup] vgButton widget.
 */
class vgButton : public Button 
{
public:
    vgButton(Widget *parent, const std::string &caption = "Untitled", int icon = 0);

    vgButton(Widget *parent, const std::string &caption, const std::function<void()> &callback)
      : vgButton(parent, caption) { setCallback(callback); }


    virtual void drawBody(SDL_Renderer* renderer) override;	
private:
  struct AsyncTexture;
  typedef std::shared_ptr<AsyncTexture> AsyncTexturePtr;
	std::vector<AsyncTexturePtr> _txs;
};

NAMESPACE_END(sdlgui)
