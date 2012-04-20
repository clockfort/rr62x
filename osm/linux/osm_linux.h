/* $Id: osm_linux.h,v 1.25 2009/07/16 01:15:17 zsf Exp $
 *
 * HighPoint RAID Driver for Linux
 * Copyright (C) 2005 HighPoint Technologies, Inc. All Rights Reserved.
 */
#ifndef _OSM_LINUX_H
#define _OSM_LINUX_H

/* system headers */

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif

#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)) && defined(MODVERSIONS)
#include <linux/modversions.h>
#endif

#include <linux/module.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
#include <linux/moduleparam.h>
#endif

#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/kdev_t.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/reboot.h>
#include <linux/random.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/div64.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <linux/blk.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#include <linux/nmi.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include "scsi.h"
#include <scsi/scsi_ioctl.h>
#else 
#include <linux/blkdev.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi.h>
typedef struct scsi_host_template Scsi_Host_Template;
typedef struct scsi_device Scsi_Device;
typedef struct scsi_cmnd Scsi_Cmnd;
typedef struct scsi_request Scsi_Request;
typedef struct scsi_pointer Scsi_Pointer;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4)
#include "hosts.h"
#else 
#include <scsi/scsi_host.h>
#endif

#pragma pack(1)
typedef struct _INQUIRYDATA {
#ifdef __BIG_ENDIAN_BITFIELD
	u8 DeviceTypeQualifier : 3;
	u8 DeviceType : 5;
	u8 RemovableMedia : 1;
	u8 DeviceTypeModifier : 7;
#else 
	u8 DeviceType : 5;
	u8 DeviceTypeQualifier : 3;
	u8 DeviceTypeModifier : 7;
	u8 RemovableMedia : 1;
#endif
	u8 Versions;
	u8 ResponseDataFormat;
	u8 AdditionalLength;
	u8 Reserved[2];
#ifdef __BIG_ENDIAN_BITFIELD
	u8 RelativeAddressing : 1;
	u8 Wide32Bit : 1;
	u8 Wide16Bit : 1;
	u8 Synchronous : 1;
	u8 LinkedCommands : 1;
	u8 Reserved2 : 1;
	u8 CommandQueue : 1;
	u8 SoftReset : 1;
#else 
	u8 SoftReset : 1;
	u8 CommandQueue : 1;
	u8 Reserved2 : 1;
	u8 LinkedCommands : 1;
	u8 Synchronous : 1;
	u8 Wide16Bit : 1;
	u8 Wide32Bit : 1;
	u8 RelativeAddressing : 1;
#endif
	u8 VendorId[8];
	u8 ProductId[16];
	u8 ProductRevisionLevel[4];
	u8 VendorSpecific[20];
	u8 Reserved3[40];
} __attribute__((packed)) INQUIRYDATA, *PINQUIRYDATA;

#pragma pack()

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,18)
#define pci_set_dma_mask(pcidev, mask) 1
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,4)
#define scsi_set_pci_device(pshost, pcidev)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define scsi_set_pci_device(pshost, pcidev)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define sc_host(sc)    sc->device->host
#define sc_channel(sc) sc->device->channel
#define sc_target(sc)  sc->device->id
#define sc_lun(sc)     sc->device->lun
#else 
#define sc_host(sc)    sc->host
#define sc_channel(sc) sc->channel
#define sc_target(sc)  sc->target
#define sc_lun(sc)     sc->lun
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
#ifndef IRQ_HANDLED
typedef void irqreturn_t;
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
#endif
#else 
#define SUGGEST_ABORT 0
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define scsi_assign_lock(host, lock)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define spin_lock_irq_io_request_lock    spin_lock_irq(&io_request_lock)
#define spin_unlock_irq_io_request_lock  spin_unlock_irq(&io_request_lock)
#define spin_lock_irqsave_io_request_lock(flags)      spin_lock_irqsave (&io_request_lock, flags)
#define spin_unlock_irqrestore_io_request_lock(flags) spin_unlock_irqrestore (&io_request_lock, flags)
#else 
#define spin_lock_irq_io_request_lock
#define spin_unlock_irq_io_request_lock
#define spin_lock_irqsave_io_request_lock(flags)
#define spin_unlock_irqrestore_io_request_lock(flags)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define scsi_set_max_cmd_len(host, len)
#else 
#define scsi_set_max_cmd_len(host, len) host->max_cmd_len = len
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define HPT_KMAP_TYPE KM_BIO_SRC_IRQ
#define HPT_FIND_PCI_DEVICE pci_get_device
#else 
#define HPT_KMAP_TYPE KM_BH_IRQ
#define HPT_FIND_PCI_DEVICE pci_find_device
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define scsi_to_pci_dma_dir(scsi_dir) ((int)(scsi_dir))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
#define hpt_verify_area(type, addr, size) (!access_ok((type), (addr), (size)))
#else 
#define hpt_verify_area(type, addr, size) (verify_area((type), (addr), (size)))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
#define HPT_SA_SHIRQ		IRQF_SHARED
#else 
#define HPT_SA_SHIRQ		SA_SHIRQ
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#define HPT_SG_PAGE(sg)		sg_page(sg)
#else 
#define HPT_SG_PAGE(sg)		(sg)->page
#endif

struct hpt_scsi_pointer {
	int mapped;
	int sgcnt;
	dma_addr_t dma_handle;
};

typedef char check_sizeof_hpt_scsi_pointer[sizeof(struct scsi_pointer)-sizeof(struct hpt_scsi_pointer)];
#define HPT_SCP(SCpnt) ((struct hpt_scsi_pointer *)&(SCpnt)->SCp)

/* private headers */

#include "osm.h"
#include "him.h"
#include "ldm.h"

/* driver parameters */
extern char driver_name[];
extern char driver_name_long[];
extern char driver_ver[];
extern int  osm_max_targets;

/*
 * adapter/vbus extensions:
 * each physical controller has an adapter_ext, passed to him.create_adapter()
 * each vbus has a vbus_ext passed to ldm_create_vbus().
 */
#define EXT_TYPE_HBA  1
#define EXT_TYPE_VBUS 2

typedef struct _hba {
	int ext_type;
	LDM_ADAPTER ldm_adapter;
	PCI_ADDRESS pciaddr;
	struct pci_dev *pcidev;
	struct _vbus_ext *vbus_ext;
	struct _hba *next;
	
	/* HIM instance continues */
	unsigned long him_handle[0] __attribute__((aligned(sizeof(unsigned long))));
}
HBA, *PHBA;

typedef struct _vbus_ext {
	int ext_type;
	PHBA hba_list;
	struct freelist *freelist_head;
	struct freelist *freelist_dma_head;
	
	/*
	 * all driver entries (except hpt_release/hpt_intr) are protected by 
	 * io_request_lock on kernel 2.4, and by host->host_lock on kernel 2.6.
	 * so, it's not necessarily to have our own spinlocks; we just need
	 * to pay attention to our proc/ioctl routines.
	 */
	spinlock_t *lock;
	struct semaphore sem;
	
	struct Scsi_Host *host;
	
	struct tasklet_struct worker;
	OSM_TASK *tasks;
	struct timer_list timer;
	
	HPT_U8 *sd_flags;
	int needs_refresh;
	
	/* the LDM vbus instance continues */
	unsigned long vbus[0] __attribute__((aligned(sizeof(unsigned long))));
}
VBUS_EXT, *PVBUS_EXT;

#define SD_FLAG_IN_USE     1
#define SD_FLAG_REVALIDATE 2
#define SD_FLAG_REMOVE     0x80

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define set_vbus_lock(vbus_ext) vbus_ext->lock = &io_request_lock
#else 
#define set_vbus_lock(vbus_ext) vbus_ext->lock = vbus_ext->host->host_lock
#endif

#define get_vbus_ext(host) (*(PVBUS_EXT *)host->hostdata)

void refresh_sd_flags(PVBUS_EXT vbus_ext);
void hpt_do_ioctl(IOCTL_ARG *ioctl_args);
void hpt_stop_tasks(PVBUS_EXT vbus_ext);
int hpt_proc_get_info(struct Scsi_Host *host, char *buffer, char **start, off_t offset, int length);

#endif
