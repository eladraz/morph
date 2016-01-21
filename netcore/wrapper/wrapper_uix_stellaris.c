#ifdef STELLARISWARE

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "drivers/kitronix320x240x16_ssd2119_8bit.h"
#include "drivers/set_pinout.h"
#include "drivers/thumbwheel.h"

tContext sContext;

#endif

volatile unsigned long g_ulSysTickCount = 0;
int thumbWheelY = 0;

#define USER_BTN_PORT  GPIO_PORTJ_BASE
#define USER_BTN_PIN   GPIO_PIN_7

void SysTickHandler(void)
{
    g_ulSysTickCount++;
    ThumbwheelTrigger();
}


static void thumbwheelCallback(unsigned short aValue) 
{
	thumbWheelY = ((3000 - aValue) * 90 / 3000);
}

int _getTickCount()
{
    return g_ulSysTickCount;
}

void _displayInit()
{
#ifdef STELLARISWARE
    //
    // Initialize the display driver.
    //
    Kitronix320x240x16_SSD2119Init();

    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sKitronix320x240x16_SSD2119);

    ThumbwheelInit();
    ThumbwheelCallbackSet(thumbwheelCallback);

    GPIODirModeSet(USER_BTN_PORT, USER_BTN_PIN, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(USER_BTN_PORT, USER_BTN_PIN, GPIO_STRENGTH_2MA,
                                GPIO_PIN_TYPE_STD_WPU);
    
    SysTickPeriodSet(SysCtlClockGet() / 1000);
    SysTickEnable();
    SysTickIntEnable();
   

    // Enable Interrupts
    IntMasterEnable();  

#elif defined(EMULATION)
    printf("_displayInit()\n");
#else 
    #error TODO! Board support packegae!
#endif
}

int _displayGetWidth()
{
#ifdef STELLARISWARE  
    return GrContextDpyWidthGet(&sContext);
#else 
    return 100;
#endif
}

int _displayGetHeight()
{
#ifdef STELLARISWARE  
    return GrContextDpyHeightGet(&sContext);
#else 
    return 100;
#endif
}

void _displayDrawRectangle(int d1, int d2, int d3, int d4,
                           int x, int y, int width, int height, int color)
{
#ifdef STELLARISWARE
    tRectangle sRect;

    sRect.sXMin = x;
    sRect.sYMin = y;
    sRect.sXMax = x + width;
    sRect.sYMax = y + height;
    GrContextForegroundSet(&sContext, color);
    GrRectDraw(&sContext, &sRect);
#elif defined(EMULATION)
    printf("_displayDrawRectangle() (%d,%d) => (%d,%d)  \n",x,y,width,height);
#else 
    #error TODO! Board support packegae!
#endif    
}

void _displayFillRectangle(int d1, int d2, int d3, int d4,
                           int x, int y, int width, int height, int color)
{
#ifdef STELLARISWARE
    tRectangle sRect;

    sRect.sXMin = x;
    sRect.sYMin = y;
    sRect.sXMax = x + width;
    sRect.sYMax = y + height;
    GrContextForegroundSet(&sContext, color);
    GrRectFill(&sContext, &sRect);
#elif defined(EMULATION)
    printf("_displayFillRectangle() (%d,%d) => (%d,%d)  \n",x,y,width,height);
#else 
    #error TODO! Board support packegae!
#endif    
}

char _displayIsKeyPressed()
{
#ifdef STELLARISWARE
    return (GPIOPinRead(USER_BTN_PORT, USER_BTN_PIN) & USER_BTN_PIN) == 0;
#elif defined(EMULATION)
    printf("_displayIsKeyPressed()\n");
#else 
    #error TODO! Board support packegae!
#endif    
}

int _displayGetPotentiometer()
{
  return thumbWheelY;
}

int _displayText(int d1, int d2, int d3, int d4, int x, int y, char* text, int len)
{
  char pcBuffer[64];
  memcpy(pcBuffer, text, len);
  pcBuffer[len] = 0;
#ifdef STELLARISWARE
  GrContextFontSet(&sContext, &g_sFontCm20);
  GrStringDraw(&sContext, pcBuffer, -1, x, y, 0);
#elif defined(EMULATION)
    printf("_displayText(): %s\n", pcBuffer);
#else 
    #error TODO! Board support packegae!
#endif    
}
