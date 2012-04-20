/* $Id: himfuncs.h,v 1.15 2008/05/27 06:32:46 gmm Exp $
 * Copyright (C) 2004-2005 HighPoint Technologies, Inc. All rights reserved.
 *
 * define _HIM_INTERFACE before include this file, and
 * undef it after include this file.
 */


#ifndef _HIM_INTERFACE
#error "you must define _HIM_INTERFACE before this file"
#endif

_HIM_INTERFACE(HPT_BOOL, get_supported_device_id, (int index, PCI_ID *id))

_HIM_INTERFACE(HPT_U8, get_controller_count, (PCI_ID *id, HPT_U8 *reached))


_HIM_INTERFACE(HPT_UINT, get_adapter_size, (const PCI_ID *id))


_HIM_INTERFACE(HPT_BOOL, create_adapter, (const PCI_ID *id, PCI_ADDRESS pciAddress, void *adapter, void *osext))

_HIM_INTERFACE(void, get_adapter_config, (void *adapter, HIM_ADAPTER_CONFIG *config))

_HIM_INTERFACE(HPT_BOOL, get_meminfo, (void *adapter))


_HIM_INTERFACE(HPT_BOOL, adapter_on_same_vbus, (void *adapter1, void *adapter2))
_HIM_INTERFACE(void, route_irq, (void *adapter, HPT_BOOL enable))


_HIM_INTERFACE(HPT_BOOL, initialize, (void *adapter))


_HIM_INTERFACE(HPT_UINT, get_device_size, (void *adapter))


_HIM_INTERFACE(HPT_BOOL, probe_device, (void *adapter, int index, void *devhandle, PROBE_CALLBACK done, void *arg))
_HIM_INTERFACE(void *, get_device, (void *adapter, int index))
_HIM_INTERFACE(void, get_device_config, (void *dev, HIM_DEVICE_CONFIG *config))
_HIM_INTERFACE(void, remove_device, (void *dev))

_HIM_INTERFACE(void, reset_device, (void * dev, void (*done)(void *arg), void *arg))


_HIM_INTERFACE(HPT_U32, get_cmdext_size, (void))

_HIM_INTERFACE(void, queue_cmd, (void *dev, struct _COMMAND *cmd))


_HIM_INTERFACE(int, read_write, (void *dev,HPT_LBA lba, HPT_U16 nsector, HPT_U8 *buffer, HPT_BOOL read))

_HIM_INTERFACE(HPT_BOOL, intr_handler, (void *adapter))
_HIM_INTERFACE(HPT_BOOL, intr_control, (void * adapter, HPT_BOOL enable))


_HIM_INTERFACE(int, get_channel_config, (void * adapter, int index, PHIM_CHANNEL_CONFIG pInfo))
_HIM_INTERFACE(int, set_device_info, (void * dev, PHIM_ALTERABLE_DEV_INFO pInfo))
_HIM_INTERFACE(void, unplug_device, (void * dev))


_HIM_INTERFACE(void, shutdown, (void *adapter))
_HIM_INTERFACE(void, suspend, (void *adapter))
_HIM_INTERFACE(void, resume, (void *adapter))
_HIM_INTERFACE(void, release_adapter, (void *adapter))

/*called after ldm_register_adapter*/
_HIM_INTERFACE(HPT_BOOL, verify_adapter, (void *adapter))

/* (optional) */
_HIM_INTERFACE(void, ioctl, (void * adapter, struct _IOCTL_ARG *arg))
_HIM_INTERFACE(int, compare_slot_seq, (void *adapter1, void *adapter2))
_HIM_INTERFACE(int, get_enclosure_count, (void *adapter))
_HIM_INTERFACE(int, get_enclosure_info, (void *adapter, int id, void *pinfo))


_HIM_INTERFACE(HPT_BOOL, flash_access, (void *adapter, HPT_U32 offset, void *value, int size, HPT_BOOL reading))

#undef _HIM_INTERFACE
