// Module Created by Avan Peltier
// Intended to enumerate PCI and return devices of the specified config type


#ifndef EnumPCI_H
#define EnumPCI_H

#include "common.h"
#include "x86arch.h"
#include "lib.h"
#include "EnumPCI.h"

struct device {
	uint8_t bus;
	uint16_t vendor;
	uint16_t device;
	uint8_t function;
	uint8_t baseClass;
	uint8_t subClass;
	uint32_t baseAdd0;
	uint32_t baseAdd1;


} typedef(Device);

void checkAllBuses(void);

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

uint32_t pciCheckVendor(uint8_t bus, uint8_t slot);

bool_t checkDevice(uint8_t bus, uint8_t device);

uint8_t getHeaderType(uint8_t bus, uint8_t device, uint8_t function);

uint16_t getVendorID(uint8_t bus, uint8_t device, uint8_t function);

uint8_t getBaseClass(uint8_t bus, uint8_t device, uint8_t function);

uint8_t getSubClass(uint8_t bus, uint8_t device, uint8_t function);

uint8_t checkFunction(uint8_t bus, uint8_t device, uint8_t function);

struct device* getDevice(void);

uint16_t pciWriteWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data);

void initPci(void);
#endif
