#include <drivers/pci.h>
#include <stdio.h>
#include <x86.h>

#define ATA_PRIMARY    0
#define ATA_SECONDARY  1

/* ATA-ATAPI Task-File: */
#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01 /* R */
#define ATA_REG_FEATURES    0x01 /* W */
#define ATA_REG_SECCOUNT0   0x02
#define ATA_REG_LBA0        0x03
#define ATA_REG_LBA1        0x04
#define ATA_REG_LBA2        0x05
#define ATA_REG_HDDEVSEL    0x06
#define ATA_REG_COMMAND     0x07 /* R */
#define ATA_REG_STATUS      0x07 /* W */

#define ATA_CTRL_CONTROL    0x00
#define ATA_CTRL_ALTSTATUS  0x00
#define ATA_CTRL_DEVADDRESS 0x01

/* Status Register */
#define ATA_SR_ERR   0x01 /* Error */
#define ATA_SR_IDX   0x02 /* Index */
#define ATA_SR_CORR  0x04 /* Corrected data */
#define ATA_SR_DRQ   0x08 /* Data request ready */
#define ATA_SR_DSC   0x10 /* Drive seek complete */
#define ATA_SR_DF    0x20 /* Drive write fault */
#define ATA_SR_DRDY  0x40 /* Drive ready */
#define ATA_SR_BSY   0x80 /* Busy */

#define ATA_CMD_READ_PIO         0x20
#define ATA_CMD_READ_PIO_EXT     0x24
#define ATA_CMD_READ_DMA         0xC8
#define ATA_CMD_READ_DMA_EXT     0x25
#define ATA_CMD_WRITE_PIO        0x30
#define ATA_CMD_WRITE_PIO_EXT    0x34
#define ATA_CMD_WRITE_DMA        0xCA
#define ATA_CMD_WRITE_DMA_EXT    0x35
#define ATA_CMD_CACHE_FLUSH      0xE7
#define ATA_CMD_CACHE_FLUSH_EXT  0xEA
#define ATA_CMD_PACKET           0xA0
#define ATA_CMD_IDENTIFY_PACKET  0xA1
#define ATA_CMD_IDENTIFY         0xEC


static uint16_t ata_baseaddr[2] = { 0x1F0, 0x170 };
static uint16_t ata_ctrladdr[2] = { 0x3F6, 0x376 };

static void ata_detect_disks(struct pci_device *dev)
{
	int chan, drive;
	uint8_t buf[1024];
	uint8_t status;
	uint16_t *wbuf;
	uint8_t *bbuf;
	int i, j;

	/* Disable interrupts */
	outb(ata_ctrladdr[0] + ATA_CTRL_CONTROL, 0x2);
	outb(ata_ctrladdr[1] + ATA_CTRL_CONTROL, 0x2);

	for (i = 0; i < 4; ++i) {
		chan = i / 2;
		drive = i % 2;

		/* Select drive */
		outb(ata_baseaddr[chan] + ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));

		/* Not sure why these writes are needed ? */
		outb(ata_baseaddr[chan] + ATA_REG_SECCOUNT0, 0x00);
		outb(ata_baseaddr[chan] + ATA_REG_LBA0, 0x00);
		outb(ata_baseaddr[chan] + ATA_REG_LBA1, 0x00);
		outb(ata_baseaddr[chan] + ATA_REG_LBA2, 0x00);

		outb(ata_baseaddr[chan] + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

		/* Wait for BSY flag to be reset */
		while (inb(ata_baseaddr[chan] + ATA_REG_STATUS) & ATA_SR_BSY);

		status = inb(ata_baseaddr[chan] + ATA_REG_STATUS);
		if (status == 0x00) {
			printf("ATA Channel %d: No %s disk detected.\n", chan, drive ? "slave" : "master");
			continue;
		}
		if (status & ATA_SR_ERR) {
			printf("ATA Channel %d: Error detecting %s disk (ATAPI?). status=0x%x.\n",
			       chan, drive ? "slave" : "master", status);
			continue;
		}

		wbuf = (uint16_t *)&buf[0];
		bbuf = &buf[512];

		for (j = 0; j < 256; ++j) {
			uint16_t data = inw(ata_baseaddr[chan] + ATA_REG_DATA);
			*wbuf++ = data;
			*bbuf++ = (data >> 8) & 0xFF;
			*bbuf++ = data & 0xFF;
		}

		wbuf = (uint16_t *)&buf[0];
		bbuf = &buf[512];

		/* Terminate and de-pad device name string */
		j = 93;
		while (j > 54 && bbuf[j] == ' ')
			bbuf[j--] = '\0';

		printf("ATA Channel %d: [%s] status=0x%x C/H/Spt=%d/%d/%d\n", chan, &bbuf[54], status, wbuf[1], wbuf[3], wbuf[6]);
	}
}

static int ata_probe(struct pci_device *dev, const struct pci_device_id *id)
{
	(void)dev;
	(void)id;

	printf("Found ATA controller at bus:%d slot:%d (vendor:%x device:%x)\n",
	       dev->bus, dev->slot, dev->vendor, dev->device);

	ata_detect_disks(dev);

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
