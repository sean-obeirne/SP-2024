/*
* Audio Driver Main File
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
	AUDIODEVICE = getDevice();
	char buf[128];
	
	soundBuffer = _km_page_alloc(62);
	AUDIODEVICE->baseAdd0 = pciConfigReadWord(AUDIODEVICE->bus, AUDIODEVICE->device, 0, 0x10) & 0xFFFFFFFC;
	int busMaster = pciConfigReadWord(AUDIODEVICE->bus, AUDIODEVICE->device, 0, 0x4);
	pciWriteWord(AUDIODEVICE->bus, AUDIODEVICE->device, 0, 0x4, busMaster | 0x4);

	__outw(AUDIODEVICE->baseAdd0 + 0x04, 0x20);

	__cio_puts("SOUND INIT");


		
}



void soundISR(int vector, int code){
	__outb(PIC_PRI_CMD_PORT, PIC_EOI);
}

void playSound(uint8_t *data, uint32_t dataLength){
	__cio_puts("playSound");
	uint8_t bitSample;
	if (AUDIODEVICE->bus == -1 ){
		return;
	}
	
	for (int i = 0; i < 128 && i < dataLength; i++){
		bitSample = data[i];
		soundBuffer[i] = bitSample;
	}

	__outw(AUDIODEVICE->baseAdd0 + 0x0c, 0x0C);
	__outw(AUDIODEVICE->baseAdd0 + 0x38, (uint32_t)soundBuffer);
	__outw(AUDIODEVICE->baseAdd0 + 0x3c, 0xFFFF);
	__outw(AUDIODEVICE->baseAdd0 + 0x28, 0x7FFF7FFF);
	__outw(AUDIODEVICE->baseAdd0 + 0x20, 0x00200204);
	__outw(AUDIODEVICE->baseAdd0 + 0x00, 0x0);
	__outw(AUDIODEVICE->baseAdd0 + 0x00, 0x00000020);

	
	__install_isr(43, soundISR);
	__cio_puts("Sound Now Playing");

}
