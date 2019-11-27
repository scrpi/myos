#include <drivers/pci.h>
#include <kernel.h>
#include <x86.h>

LIST_HEAD(pci_device_list);
LIST_HEAD(pci_driver_list);

static uint32_t pci_read_config_data(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint32_t address;
	uint32_t val;

	offset &= ~0x3;

	address = ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
	          ((uint32_t)func << 8) | offset | ((uint32_t)0x80000000);

	outl(0xCF8, address);

	val = inl(0xCFC);

	return val;
}

static uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint16_t val;
	int shift;

	shift = (offset & 0x2) * 8;

	val = (pci_read_config_data(bus, slot, func, offset) >> shift) & 0xFFFF;

	return val;
}

static uint16_t pci_read_byte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
	uint8_t val;
	int shift;

	shift = (offset & 0x3) * 8;

	val = (pci_read_config_data(bus, slot, func, offset) >> shift) & 0xFF;

	return val;
}

static void probe_pci_function(uint8_t bus, uint8_t slot, uint8_t func)
{
	struct pci_device *dev;

	dev = malloc(sizeof(*dev));
	if (!dev)
		panic("OOM");

	memset(dev, 0, sizeof(*dev));

	dev->bus = bus;
	dev->slot = slot;
	dev->func = func;

	dev->vendor = pci_read_word(bus, slot, func, PCI_CFG_VENDOR_ID);
	dev->device = pci_read_word(bus, slot, func, PCI_CFG_DEVICE_ID);

	dev->base_class = pci_read_byte(bus, slot, func, PCI_CFG_CLASSCODE);
	dev->sub_class  = pci_read_byte(bus, slot, func, PCI_CFG_SUBCLASS);

	list_add(&dev->node, &pci_device_list);
}

static void probe_pci_dev(uint8_t bus, uint8_t slot)
{
	uint16_t header_type;
	uint16_t vendor;
	uint8_t function = 0;

	/* Check vendor ID */
	vendor = pci_read_word(bus, slot, 0, PCI_CFG_VENDOR_ID);
	if (vendor == 0xFFFF)
		return; /* No device present */

	probe_pci_function(bus, slot, function);

	header_type = pci_read_byte(bus, slot, 0, PCI_CFG_HEADER_TYPE);

	if (header_type & PCI_HEADER_TYPE_MULTIFUNC) {
		for (function = 1; function < 8; function++) {
			vendor = pci_read_word(bus, slot, function, PCI_CFG_VENDOR_ID);

			if (vendor == 0xFFFF)
				continue;

			probe_pci_function(bus, slot, function);
		}
	}
}

static void pci_scan()
{
	uint16_t bus;
	uint8_t slot;

	for (bus = 0; bus < 256; ++bus) {
		for (slot = 0; slot < 32; ++slot) {
			probe_pci_dev(bus, slot);
		}
	}
}

static void do_driver_probe()
{
	const struct pci_device_id *id;
	struct pci_device *dev;
	struct pci_driver *driver;

	list_for_each_entry(dev, &pci_device_list, node) {
		list_for_each_entry(driver, &pci_driver_list, node) {
			if (!driver->probe)
				continue;

			id = driver->id_table;

			while (id->vendor != 0) {
				if (dev->vendor != id->vendor && id->vendor != PCI_ANY_ID)
					goto next_id;
				if (dev->device != id->device && id->device != PCI_ANY_ID)
					goto next_id;
				if (dev->base_class != id->base_class && id->base_class != PCI_ANY_ID)
					goto next_id;
				if (dev->sub_class != id->sub_class && id->sub_class != PCI_ANY_ID)
					goto next_id;

				if (0 == driver->probe(dev, id)) {
					/* Found driver */
					dev->driver = driver;
					break;
				}
next_id:
				++id;
			}
		}
	}
}

void pci_init()
{
	struct pci_device *dev;

	pci_scan();

	list_for_each_entry_reverse(dev, &pci_device_list, node) {
		printf("PCI Device [%d,%d,%d] %x:%x %d:%d\n", dev->bus, dev->slot, dev->func,
		       dev->vendor, dev->device, dev->base_class, dev->sub_class);
	}

	/* TODO: Replace with some kind of gcc constructor mechanism? */

	do_driver_probe();
}

void pci_register_driver(struct pci_driver *driver)
{
	list_add(&driver->node, &pci_driver_list);

	printf("Registered PCI Driver: %s\n", driver->name);
}
