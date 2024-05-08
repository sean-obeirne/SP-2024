#include "EnumPCI.h"
#include "common.h"
#include "cio.h"
#include "kmem.h"

struct device *connectedDevices;
int numDevices = 0;

void initPci(void){
	checkAllBuses();
}
//Iterate all the buses
void checkAllBuses(){
	uint16_t bus;
	uint8_t device;
	bool_t check = false;

	for (bus = 0; bus < 256; bus++){
		for (device = 0; device < 32; device++){
			check = checkDevice(bus, device);
			if (check){
				return;
			}
		}
	}
}
//Write word into address
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
//OS Dev function for PCI configuration space
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
//Legacy function from OSDev Wiki didn´t need it for my use
uint32_t pciCheckVendor(uint8_t bus, uint8_t slot){
	uint32_t vendor, device;
	vendor = pciConfigReadWord(bus, slot, 0, 0);
	if (vendor != 0xFFFF){
		device = pciConfigReadWord(bus, slot, 0, 2);
	}
	return vendor;
}
//Check device
bool_t checkDevice(uint8_t bus, uint8_t device){
	uint8_t function = 0;
	uint8_t check = 0;
	uint8_t vendorID = getVendorID(bus, device, function);
	if (vendorID == 0xFF){ 
		return false;
	}
	//__cio_printf("Vendor: %x\n", vendorID);

	function = checkFunction(bus,device,function);
	uint8_t headerType = getHeaderType(bus, device, function);
	//__cio_printf("HeaderType: %x\n", headerType);

	//if ((headerType & 0x80) != 0){
		for (function = 1; function < 8; function++){
			//if (getVendorID(bus, device, function) != 0xFFFF){
				check = checkFunction(bus, device, function);
				if (check == 0x1){
					return true;
				//}
			}
		}
	
	//}
	return false;
}



uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xE);
	tmp = ((tmp >> ((0xE & 1) * 8)) & 0xFFFF);
	//DO NOT BITSHIFT TWICE FOR NO REASON
	return tmp;
}

uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0x0);
	tmp = ((tmp >> ((0x0 & 0x01) * 8)) & 0xFFFF);
	//DO NOT BITSHIFT TWICE FOR NO REASON
	return tmp;

}


uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xB);
	tmp = ((tmp >> ((0xB & 1) * 8)) & 0xFFFF);
	//DO NOT BITSHIFT TWICE FOR NO REASON
	return tmp;

}

uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function){
	uint32_t tmp = pciConfigReadWord(bus, device, function, 0xA);
	tmp = ((tmp >> ((0xA & 1) * 8)) & 0xFFFF);
	//DO NOT BITSHIFT TWICE FOR NO REASON
	return tmp;

}


//For each function see if class and subclass are audio
uint8_t checkFunction(uint8_t bus, uint8_t device, uint8_t function) {
	uint8_t baseClass;
	uint8_t subClass;

	baseClass = getBaseClass(bus, device, function);
	//__cio_printf("\n%x\n",baseClass);
	subClass = getSubClass(bus, device, function);
	uint32_t baseAdd0 = (pciConfigReadWord(bus, device, function, 0x10) & 0xFFFFFFFC);
	uint32_t baseAdd1 = (pciConfigReadWord(bus, device, function, 0x14) & 0xFFFFFFFC) ;
	uint16_t vendor = getVendorID(bus, device, function);
	//__cio_printf("Vendor: %x\n", vendor);
	if (baseClass == 0x4 && subClass == 0x3){
		struct device *audioDevice = _km_page_alloc(sizeof(struct device));
		audioDevice->bus = bus;
		audioDevice->device = device;
		audioDevice->function = function;
		audioDevice->baseClass = baseClass;
		audioDevice->subClass = subClass;
		audioDevice->baseAdd0 = baseAdd0;
		audioDevice->baseAdd1 = baseAdd1;
		audioDevice->vendor = vendor;
		connectedDevices= audioDevice;
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
		return connectedDevices;
	}
	else {
		return connectedDevices;
	}
}
