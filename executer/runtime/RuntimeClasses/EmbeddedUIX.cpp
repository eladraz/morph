/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

/*
 * EmbeddedUIX.cpp
 *
 * Implementation file
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "executer/stdafx.h"
#include "xStl/types.h"
#include "xStl/stream/ioStream.h"
#include "executer/runtime/RuntimeClasses/EmbeddedUIX.h"
#include "GWS/MotifStyle/skin.h"

ClrWindow::ClrWindow()
{
    m_ptPosition = (PT_MIN + PT_MAX) /2;
}

void ClrWindow::onChar(uint32 flags, gchar character)
{
    // Count keys
    if (character == GWS_VK_UP)
        if (m_ptPosition < PT_MAX)
            m_ptPosition++;

    if (character == GWS_VK_DOWN)
        if (m_ptPosition > PT_MIN)
            m_ptPosition--;
}

bool ClrWindow::isKeyPressed()
{
#ifndef XSTL_WINDOWS
    // TODO!
    CHECK_FAIL();
    return false;
#else
    return (GetAsyncKeyState(VK_SPACE) != 0);
#endif
}

uint ClrWindow::getPT()
{
    return m_ptPosition;
}

///////////////

ClrWindowApp::~ClrWindowApp()
{
}

void ClrWindowApp::createSkin()
{
    m_skin = cSmartPtr<cSkin>((cSkin*)(new cMotifSkin()));
}

bool ClrWindowApp::initInstance()
{
    ASSERT(!m_skin.isEmpty());

    // Wrap the object as stack object
    m_clrWindow = new ClrWindow();
    m_mainWindow = cSmartPtr<cWnd>(m_clrWindow);

    /* Create main window */
    m_clrWindow->create(m_display,
                        NULL,
                        m_skin,
                        "M0rph - (C) Integrity Project",
                        cWnd::getCenterLocation(m_display, EmbeddedUIX::DISPLAY_WIDTH, EmbeddedUIX::DISPLAY_HEIGHT));
    m_readyEvent.setEvent();
    return true;
}

void ClrWindowApp::run()
{
    // Call windows OS Main loop. See app_win_os.cpp
    // TODO! Add Linux code here as well...
    WinMain(NULL, NULL, NULL, 1);

    // Someone press ALT-F4. TODO! Define behaviour...
}

void ClrWindowApp::waitForReadyThread()
{
    m_readyEvent.wait();
}

/////////////////////////////////////////////////////////////////////////////////


ClrWindowApp* EmbeddedUIX::gTheApp = NULL;

void EmbeddedUIX::initClrUIX()
{
    if (gTheApp == NULL)
    {
        gTheApp = new ClrWindowApp();
        gTheApp->start();
        gTheApp->waitForReadyThread();
        gTheApp->m_mainWindow->waitUntilCreated();
    }
    // else twice call to init, clear window
}

void EmbeddedUIX::drawRectangle(int x, int y, int width, int height, int color)
{
    if (gTheApp != NULL)
    {
        cDC dc = gTheApp->m_mainWindow->getDC();

        dc.setForegroundColor(cColor((color >> 16) & 0xFF,
                                     (color >>  8) & 0xFF,
                                     (color >>  0) & 0xFF));
        dc.drawRect(x,y, x+width, y+height);
    }
}

void EmbeddedUIX::fillRectangle(int x, int y, int width, int height, int color)
{
    if (gTheApp != NULL)
    {
        cDC dc = gTheApp->m_mainWindow->getDC();

        dc.setForegroundColor(cColor((color >> 16) & 0xFF,
                                     (color >>  8) & 0xFF,
                                     (color >>  0) & 0xFF));
        dc.fillRect(x,y, x+width, y+height);
    }
}

uint EmbeddedUIX::getWidth()
{
    return EmbeddedUIX::DISPLAY_WIDTH;
}

uint EmbeddedUIX::getHeight()
{
    return EmbeddedUIX::DISPLAY_HEIGHT;
}

uint EmbeddedUIX::getPotentiometer()
{
    if (gTheApp != NULL)
    {
        return gTheApp->m_clrWindow->getPT();
    }
    return 0;
}

bool EmbeddedUIX::isKeyPressed()
{
    if (gTheApp != NULL)
    {
        return gTheApp->m_clrWindow->isKeyPressed();
    }
    return false;
}

void EmbeddedUIX::text(int x, int y, char* text, uint length)
{
    if (gTheApp != NULL)
    {
        cDC dc = gTheApp->m_mainWindow->getDC();

        char* nstring = new char[length + 1];
        memcpy(nstring, text, length);
        nstring[length] = 0;
        cString clrString(nstring);
        delete[] nstring;
        dc.textout(x,y, clrString);
    }
}
