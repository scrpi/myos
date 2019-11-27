#ifndef __KERNEL_DRIVERS_PCI_H_
#define __KERNEL_DRIVERS_PCI_H_

#include <stdint.h>
#include <lib/list.h>

#define PCI_CFG_VENDOR_ID       0
#define PCI_CFG_DEVICE_ID       2
#define PCI_CFG_SUBCLASS        10
#define PCI_CFG_CLASSCODE       11
#define PCI_CFG_HEADER_TYPE     14

#define PCI_HEADER_TYPE_GENERAL    0x00
#define PCI_HEADER_TYPE_BRIDGE     0x01
#define PCI_HEADER_TYPE_CARDBUS    0x02
#define PCI_HEADER_TYPE_MULTIFUNC  0x80

struct pci_driver;

struct pci_device {
	struct list_head node;
	uint8_t bus;
	uint8_t slot;
	uint8_t func;

	uint16_t vendor;
	uint16_t device;

	uint8_t base_class;
	uint8_t sub_class;

	struct pci_driver *driver;
};

#define PCI_ANY_ID ((uint32_t)-1)

struct pci_device_id {
	uint32_t vendor;
	uint32_t device;
	uint32_t base_class;
	uint32_t sub_class;
	uint32_t priv;
};

struct pci_driver {
	struct list_head node;
	const char *name;
	const struct pci_device_id *id_table;
	int (*probe)(struct pci_device *dev, const struct pci_device_id *id);
};

void pci_init();
void pci_register_driver(struct pci_driver *driver);

#endif
