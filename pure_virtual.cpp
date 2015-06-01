/*
 * pure_virtual.cpp
 *
 *  Created on: 28/11/2012
 *      Author: Andre
 */

extern "C" void __cxa_pure_virtual()
{
#ifdef __AVR__
    asm("cli");
#endif
    while (1)
	;
}

