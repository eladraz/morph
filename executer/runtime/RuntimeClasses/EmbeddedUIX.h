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

#ifndef __TBA_CLR_EXECUTER_RUNTIMECLASSES_EMBEDDEDUIX_H
#define __TBA_CLR_EXECUTER_RUNTIMECLASSES_EMBEDDEDUIX_H


/*
 * Embedded.h
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/threadedClass.h"
#include "GWS/view.h"
#include "GWS/application.h"
#include "GWS/mainwnd.h"
#include "GWS/MotifStyle/mainwnd.h"

#pragma warning(push)
#pragma warning(disable:4200)

/*
 * See GWS::cView for cross-platform UI system
 */
class ClrWindow : public cMotifMainWindow
{
public:
    enum { PT_MIN = 0, PT_MAX=100 };

    // Constructor
    ClrWindow();

    /*
     * Draws a big circle over the screen.
     *
     * dc - The device content of the drawing window. This object constructed
     *      by the operation system and it's should be used for right clipping.
     */
    virtual void onChar(uint32 flags, gchar character);

    /*
     * Return true if space key is pressed
     */
    bool isKeyPressed();

    /*
     * Return the position
     */
    uint getPT();

private:
    uint m_ptPosition;
};

/*
 * UIX Application
 *
 * Author: Elad Raz
 */
class ClrWindowApp : public cApplication,
                     public cThreadedClass
{
public:
    // Virtual functions
    virtual ~ClrWindowApp();

    /*
     * Call GWS windows proc
     */
    virtual void run();

    /*
     * Callback function. Called to init the m_skin member for the application
     * requested skin.
     */
    virtual void createSkin();

    /*
     * Callback function. Called when the application is start to be loading
     * an instance. The programmer should overload this function and init
     * the application.
     *
     * The function create a simple viewport window.
     *
     * Returns 'true'  if the application succed with the initilaize and message
     *                 loop should be created.
     * Returns 'false' if the application faild and the application should terminate
     *                 the function 'exitInstance()' will still be called.
     */
    virtual bool initInstance();

    /*
     * Wait until window is fully created.
     */
    void waitForReadyThread();

    // Access special functions
    ClrWindow* m_clrWindow;

private:
    cEvent m_readyEvent;
};


/*
 * Author: Elad Raz
 */
class EmbeddedUIX {
public:

    enum { DISPLAY_WIDTH  = 800,
           DISPLAY_HEIGHT = 600 };

    /*
     * Start the UIX window
     */
    static void initClrUIX();

    // Draw a rectangle
    static void drawRectangle(int x, int y, int width, int height, int color);

    // Draw a filled the rectangle
    static void fillRectangle(int x, int y, int width, int height, int color);

    // Return the width of the display
    static uint getWidth();

    // Return the height of the display
    static uint getHeight();

    // Return the position of the potentiometer
    static uint getPotentiometer();

    // Return if Space is pressed
    static bool isKeyPressed();

    // Display a text on the screen
    static void text(int x, int y, char* text, uint length);

    /*
     *
     */
    // static void stopClrUIX();

private:
    static ClrWindowApp *gTheApp;
};

#endif // __TBA_CLR_EXECUTER_RUNTIMECLASSES_EMBEDDEDUIX_H

