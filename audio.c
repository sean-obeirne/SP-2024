/*
* Audio Driver Main File
* Note to the reader: I have no clue what some of the register code is actually doinh
* any comments are what I´m attempting to do
* Author: Avan Peltier
*/

#include "audio.h"
#include "lib.h"
#include "EnumPCI.h"
#include "common.h"
#include "cio.h"
#include "kmem.h"
#include "support.h"
#include "x86pic.h"

// GLOBALS

struct device *AUDIODEVICE;
uint8_t *soundBuffer;


void initAudio(){
	//Malloc space for audioDevice
	AUDIODEVICE = _km_page_alloc(sizeof(struct device));
	
	AUDIODEVICE = getDevice();
	//Make soundBuffer == 128 bits
	soundBuffer = _km_page_alloc(sizeof(uint8_t)*128);
	
	//Write CORB
	__outw(AUDIODEVICE->baseAdd0 + 0x4, 0x20);

	
		
}



void soundISR(int vector, int code){
	__outb(PIC_PRI_CMD_PORT, PIC_EOI);
}

void playSound(uint8_t *data, uint32_t dataLength){
	uint8_t bitSample;
	if (AUDIODEVICE->bus == -1 ){
		return;
	}

	for (int i = 0; i < 128 && i < dataLength; i++){
		bitSample = data[i];
		soundBuffer[i] = bitSample;
	}

	//Testing PCI Connection
	//__cio_printf("\n%x\n", AUDIODEVICE->baseClass);
	//__cio_printf("\n%x\n", AUDIODEVICE->subClass);
	//__cio_printf("%x", AUDIODEVICE->device);
	//__cio_printf("%x", AUDIODEVICE->baseAdd0);
	//__cio_printf("%x", AUDIODEVICE->vendor);

	//Attempt to connect to CORB register	
	__outw(AUDIODEVICE->baseAdd0 + 0x0c, 0x0C);
	//Attempt to connect to write Data to output register
	__outw(AUDIODEVICE->baseAdd0 + 0x38, (uint32_t)soundBuffer);
	//Block data sending while playing
	__outw(AUDIODEVICE->baseAdd0 + 0x3c, 0xFFFF);

	// Attempt to get the register to output this is where the interrupt occurs
	//__outw(AUDIODEVICE->baseAdd0 + 0x28, 0x7FFF7FFF);
	//__outw(AUDIODEVICE->baseAdd0 + 0x20, 0x00200204);

	//Clear CORB
	__outw(AUDIODEVICE->baseAdd0 + 0x00, 0x0);
	__outw(AUDIODEVICE->baseAdd0 + 0x00, 0x00000020);

	//Add interrupt handler to interrupt  table
	__install_isr(43, soundISR);
	__cio_puts("Sound Now Playing");

}
