#include "EnumPCI.h"
#include "common.h"
#include "cio.h"

static struct device **connectedDevices;
static int numDevices = 0;

void initPci(void){
	__cio_puts("PCI Initiated");
	checkAllBuses();
	__cio_printf("%d", connectedDevices[0][0]);
}

void checkAllBuses(){
	uint16_t bus;
	uint8_t device;

	for (bus = 0; bus < 256; bus++){
		for (device = 0; device < 32; device++){
			checkDevice(bus, device);	
		}
	}
}

uint16_t pciWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data){
	uint32_t address;
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	
	address = (uint32_t)((lbus << 16) | (lslot << 11) |
			(lfunc << 8) | (offset & 0xFC) | (uint32_t)(0x80000000));

	__outl(0xCF8, address);
	__outl(0xCFC, data);
}

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
	uint32_t address;
	uint32_t lbus = (uint32_t)bus;
	uint32_t lslot = (uint32_t)slot;
	uint32_t lfunc = (uint32_t)func;
	uint32_t tmp = 0;
	
	address = (uint32_t)((lbus << 16) | (lslot << 11) |
			(lfunc << 8) | (offset & 0xFC) | (uint32_t)(0x80000000));
	__outl(0xCF8, address);
	tmp = (uint16_t)((__inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
	return tmp;

}

uint32_t pciCheckVendor(uint8_t bus, uint8_t slot){
	uint32_t vendor, device;
	vendor = pciConfigReadWord(bus, slot, 0, 0);
	if (vendor != 0xFFFF){
		device = pciConfigReadWord(bus, slot, 0, 2);
	}
	return vendor;
}

bool_t checkDevice(uint8_t bus, uint8_t device){
	uint8_t function = 0;
	uint8_t vendorID = getVendorID(bus, device, function);
	if (vendorID == 0xFFFF){ 
		return false;
		}
	function = checkFunction(bus,device,function);
	uint8_t headerType = getHeaderType(bus, device, function);
	if ((headerType & 0x80) != 0){
		for (function = 1; function < 8; function++){
			if (getVendorID(bus, device, function) != 0xFFFF){
				checkFunction(bus, device, function);
			}
		}
		if (numDevices > 0){
			return true;
		}
	}
	return false;
}



uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xE);
	tmp = ((tmp >> ((0xE & 1) * 8)) & 0xFFFF);
	return (tmp >> ((0xE & 1) * 8) & 0xFF);
}

uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0);
	tmp = ((tmp >> ((0 & 1) * 8)) & 0xFFFF);
	return (tmp >> ((0 & 1) * 8) & 0xFF);

}


uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xB);
	tmp = ((tmp >> ((0xB & 1) * 8)) & 0xFFFF);
	return (tmp >> ((0xB & 1) * 8) & 0xFF);

}

uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xE);
	tmp = ((tmp >> ((0xA & 1) * 8)) & 0xFFFF);
	return (tmp >> ((0xA & 1) * 8) & 0xFF);

}

uint8_t checkFunction(uint8_t bus, uint8_t device, uint8_t function) {
	uint8_t baseClass;
	uint8_t subClass;

	baseClass = getBaseClass(bus, device, function);
	subClass = getSubClass(bus, device, function);
	uint32_t baseAdd0 = pciConfigReadWord(bus, device, function, 0x10);
	uint32_t baseAdd1 = pciConfigReadWord(bus, device, function, 0x14);
	uint16_t vendor = pciCheckVendor(bus, device);
	if (baseClass == 0x4 && subClass == 0x3){
		struct device *audioDevice;
		audioDevice->bus = bus;
		audioDevice->device = device;
		audioDevice->function = function;
		audioDevice->baseClass = baseClass;
		audioDevice->subClass = subClass;
		audioDevice->baseAdd0 = baseAdd0;
		audioDevice->baseAdd1 = baseAdd1;
		audioDevice->vendor = vendor;

		connectedDevices[numDevices] = audioDevice;
		numDevices += 1;
		return 0x1;
	}
	else{
		return 0x0;
	
	}
}
struct device* getDevice(){
	if (numDevices == 0){
		initPci();
		return connectedDevices[0];
	}
	else {
		return connectedDevices[0];
	}
}
