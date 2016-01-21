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
#else 
  #include <stdio.h>
#endif // STELLARISWARE

#ifdef LINUX_VER
#define __stdcall __attribute__((stdcall))
#elif defined(__CORE__)
// Preprocessor of IAR compiler. We are now compiling for ARM
#define __stdcall 
#define IAR_VER
#endif


typedef void (__stdcall * FUNCTYPEARG)(void* arg);

#if defined(LINUX_VER) || defined(IAR_VER)
void _assertFalse()
#else
void assertFalse()
#endif // LINUX_VER
{
#ifdef EMULATION
    printf("asertFalse()!\n");
#endif
}

#if defined(IAR_VER) || defined(__arm__)
void _unhandledException(int d1, int d2, int d3, int d4, 
                         void* e)
#elif LINUX_VER 
void _unhandledException(void* e)
#else
void unhandledException(void* e)
#endif // LINUX_VER
{
}

#if defined(IAR_VER) || defined(__arm__)
void _debugChar(
                int d1, int d2, int d3, int d4, 
                int ch)
#elif LINUX_VER
void _debugChar(int ch)
#else
void debugChar(int ch)
#endif // LINUX_VER
{
#ifndef STELLARISWARE
    putchar(ch);
#endif // STELLARISWARE
}

#if defined(IAR_VER) || defined(__arm__)
int p = 0;

// The unused arguments are for tricking the compiler to use stdcall (e.g. params via stack arguments)
// Thumb has less arguments
void _callFunction(
                   int d1, int d2, int d3, int d4, 
//#if defined(__thumb__)
//                   int d1, int d2, int d3,
//#else
//                   int d1, int d2, int d3, int d4, 
//#endif
                   FUNCTYPEARG handler, void* param)
{  
#if defined(__thumb__)
    // For THUMB architecture, function pointers lower bit should be 1
    handler = (FUNCTYPEARG)(((unsigned int)handler + 1));
    asm("PUSH {R0}");    
    handler(param); 
    asm("ADD SP, SP, #0x04");    
#else 
    handler = (FUNCTYPEARG)(((unsigned int)handler));
    p = (int)param;
    asm("STR R0, [SP, #-4]!");
    handler(param); 
    asm("LDR R0, [SP], #4");  
#endif
    
}
#else
    #ifdef LINUX_VER
        void _callFunction(FUNCTYPEARG handler, void* param)
    #else
        void callFunction(FUNCTYPEARG handler, void* param)
    #endif // LINUX_VER
    {
        handler(param);
    }
#endif

#ifdef EMULATION
int totalMemory = 0;
#endif

#if defined(IAR_VER) || defined(__arm__)
void * _malloc(int d1, int d2, int d3, int d4, 
               unsigned int s) 
{
#ifdef EMULATION
  // Adding allocation length of buffer
  char* ret = (char*)malloc(s+4); 
  *((unsigned int*)ret) = s;
  totalMemory+= s;
  return (void*)(ret+4);
#else
    void* ret = (void*)malloc(s);
    return ret;
#endif
}              
#elif LINUX_VER
void * _malloc(size_t s) 
{
    return malloc(s);
}
#endif

#if defined(IAR_VER) || defined(__arm__)
void _free(int d1, int d2, int d3, int d4, 
             void * p) 
{
#ifdef EMULATION
  int* p1 = ((int*)p) - 1;
  totalMemory-= *p1;
  free(p1);
#else
   free(p);
#endif
}               
#elif LINUX_VER
void _free(void * p) {
	free(p);
}
#endif // LINUX_VER

// Application main export
extern void _myMain();
void boardInit();

#if defined(LINUX_VER) || defined(__CORE__)
int main()
{
    boardInit();
    _myMain();
} 
#else
int __stdcall WinMain(void* hInstance,
				      void* hPrevInstance,
				 	  void* lpCmdLine,
				      int nCmdShow)
{
	myMain();
}
#endif

void boardInit()
{
// For stellaris, prepare board APIs
#ifdef STELLARISWARE
    //
    // Set the system clock to run at 50MHz from the PLL
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    //
    // Initialize the device pinout appropriately for this board.
    //
    PinoutSet();
#endif
}
