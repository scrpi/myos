#include <drivers/pci.h>
#include <stdio.h>

static int ata_probe(struct pci_device *dev, const struct pci_device_id *id)
{
	(void)dev;
	(void)id;

	printf("Found ATA controller at bus:%d slot:%d (vendor:%x device:%x)\n",
	       dev->bus, dev->slot, dev->vendor, dev->device);

	return 0;
}

static struct pci_device_id ata_table[] = {
	{ PCI_ANY_ID, PCI_ANY_ID, 0x01, 0x01, 0 },
	{ 0 },
};

static struct pci_driver ata_driver = {
	.name = "Generic ATA Controller",
	.id_table = ata_table,
	.probe = ata_probe,
};

void ata_driver_init()
{
	pci_register_driver(&ata_driver);
}
