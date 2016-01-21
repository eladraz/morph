// 32c_wrapper.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include "imports.h"

void debugChar(int ch)
{
	putchar(ch);
}

void assertFalse()
{
}

void callFunction(FUNCTYPEARG handler, void* param)
{
	handler(param);
}

extern void myMain();

int main(int argc, char** argv[])
{
	myMain();
	return 0;
}
