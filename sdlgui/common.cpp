/*
    sdlgui/common.cpp -- Basic initialization and utility routines

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/screen.h>
#if defined(_WIN32)
#include <windows.h>
#endif

#ifdef SDLGUI_LINUX
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
    #include <SDL_image.h>
    #include "nanovg.h"
#endif

#include <map>
#include <array>
#include <thread>
#include <chrono>
#include <iostream>

#if !defined(_WIN32)
    #include <locale.h>
    #include <signal.h>
    #include <sys/dir.h>
#endif

NAMESPACE_BEGIN(sdlgui)

NVGcolor Color::toNvgColor() const {
  return reinterpret_cast<const NVGcolor &>(this->_d);
}

extern std::map<SDL_Window *, Screen *> __sdlgui_screens;

SDL_FPoint to_fpnt(const SDL_Point& p) { return SDL_FPoint{ (float)p.x, (float)p.y }; }

PntRect srect2pntrect(const SDL_Rect& srect)
{
  return PntRect{ srect.x, srect.y, srect.x + srect.w, srect.y + srect.h };
}

PntRect srect2pntrect(const SDL_FRect& srect)
{
  return { (int)srect.x, (int)srect.y, (int)srect.x + (int)srect.w, (int)srect.y + (int)srect.h };
}

PntFRect sfrect2pntfrect(const SDL_FRect& srect)
{
  return{ srect.x, srect.y, srect.x + srect.w, srect.y + srect.h };
}

SDL_FRect pntfrect2srectf(const PntFRect& frect)
{
  return{ frect.x1, frect.y1, frect.x2 - frect.x1, frect.y2 - frect.y1 };
}

SDL_FRect pntrect2srectf(const PntRect& frect)
{
  return{ (float)frect.x1, (float)frect.y1, (float)frect.x2 - (float)frect.x1, (float)frect.y2 - (float)frect.y1 };
}

SDL_FRect clip_rects(SDL_FRect af, const SDL_FRect& bf)
{
  PntFRect a{ af.x, af.y, af.x + af.w, af.y + af.h };
  PntFRect b{ bf.x, bf.y, bf.x + bf.w, bf.y + bf.h };
  if (a.x1 < b.x1)
    a.x1 = b.x1;
  if (a.y1 < b.y1)
    a.y1 = b.y1;
  if (b.x2 < a.x2)
    a.x2 = b.x2;
  if (b.y2 < a.y2)
    a.y2 = b.y2;

  return { a.x1, a.y1, a.x2 - a.x1, a.y2 - a.y1 };
}

PntRect clip_rects(PntRect a, const PntRect& b)
{
  if (a.x1 < b.x1)
    a.x1 = b.x1;
  if (a.y1 < b.y1)
    a.y1 = b.y1;
  if (b.x2 < a.x2)
    a.x2 = b.x2;
  if (b.y2 < a.y2)
    a.y2 = b.y2;

  return a;
}

SDL_Color Color::toSdlColor() const
{
  SDL_Color color{ r() * 255, g() * 255, b() * 255, a() * 255 };
  return color;
}

std::array<char, 8> utf8(int c) {
    std::array<char, 8> seq;
    int n = 0;
    if (c < 0x80) n = 1;
    else if (c < 0x800) n = 2;
    else if (c < 0x10000) n = 3;
    else if (c < 0x200000) n = 4;
    else if (c < 0x4000000) n = 5;
    else if (c <= 0x7fffffff) n = 6;
    seq[n] = '\0';
    switch (n) {
        case 6: seq[5] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x4000000;
        case 5: seq[4] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x200000;
        case 4: seq[3] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x10000;
        case 3: seq[2] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x800;
        case 2: seq[1] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0xc0;
        case 1: seq[0] = c;
    }
    return seq;
}

#if !defined(__APPLE__)
std::string file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save) {
#define FILE_DIALOG_MAX_BUFFER 1024
#if defined(_WIN32)
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    char tmp[FILE_DIALOG_MAX_BUFFER];
    ofn.lpstrFile = tmp;
    ZeroMemory(tmp, FILE_DIALOG_MAX_BUFFER);
    ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
    ofn.nFilterIndex = 1;

    std::string filter;

    if (!save && filetypes.size() > 1) {
        filter.append("Supported file types (");
        for (size_t i = 0; i < filetypes.size(); ++i) {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.append(")");
        filter.push_back('\0');
        for (size_t i = 0; i < filetypes.size(); ++i) {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.push_back('\0');
    }
    for (auto pair: filetypes) {
        filter.append(pair.second);
        filter.append(" (*.");
        filter.append(pair.first);
        filter.append(")");
        filter.push_back('\0');
        filter.append("*.");
        filter.append(pair.first);
        filter.push_back('\0');
    }
    filter.push_back('\0');
    ofn.lpstrFilter = filter.data();

    if (save) {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        if (GetSaveFileNameA(&ofn) == FALSE)
            return "";
    } else {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileNameA(&ofn) == FALSE)
            return "";
    }
    return std::string(ofn.lpstrFile);
#else
    char buffer[FILE_DIALOG_MAX_BUFFER];
    std::string cmd = "/usr/bin/zenity --file-selection ";
    if (save)
        cmd += "--save ";
    cmd += "--file-filter=\"";
    for (auto pair: filetypes)
        cmd += "\"*." + pair.first +  "\" ";
    cmd += "\"";
    FILE *output = popen(cmd.c_str(), "r");
    if (output == nullptr)
        throw std::runtime_error("popen() failed -- could not launch zenity!");
    while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
        ;
    pclose(output);
    std::string result(buffer);
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return result;
#endif
}
#endif

void Object::decRef(bool dealloc) const noexcept {
    --m_refCount;
    if (m_refCount == 0 && dealloc) {
        delete this;
    } else if (m_refCount < 0) {
        fprintf(stderr, "Internal error: Object reference count < 0!\n");
        abort();
    }
}

Object::~Object() { }

NAMESPACE_END(sdlgui)

