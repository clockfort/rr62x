/* $Id: hptinfo.c,v 1.24 2008/09/05 03:30:17 xxj Exp $
 *
 * HighPoint RAID Driver for Linux
 * Copyright (C) 2005 HighPoint Technologies, Inc. All Rights Reserved.
 *
 * hptinfo.c - output info via /proc filesystem.
 */
#include "osm_linux.h"
#include "hptintf.h"

typedef struct _hpt_GET_INFO {
	char *buffer;
	int buflength;
	int bufoffset;
	int buffillen;
	int filpos;
} HPT_GET_INFO;

static void hpt_copy_mem_info(HPT_GET_INFO *pInfo, char *data, int datalen)
{
	if (pInfo->filpos < pInfo->bufoffset) {
		if (pInfo->filpos + datalen <= pInfo->bufoffset) {
			pInfo->filpos += datalen;
			return;
		} else {
			data += (pInfo->bufoffset - pInfo->filpos);
			datalen  -= (pInfo->bufoffset - pInfo->filpos);
			pInfo->filpos = pInfo->bufoffset;
		}
	}

	pInfo->filpos += datalen;
	if (pInfo->buffillen == pInfo->buflength)
		return;

	if (pInfo->buflength - pInfo->buffillen < datalen)
		datalen = pInfo->buflength - pInfo->buffillen;

	memcpy(pInfo->buffer + pInfo->buffillen, data, datalen);
	pInfo->buffillen += datalen;
}

static int hpt_copy_info(HPT_GET_INFO *pinfo, char *fmt, ...)
{
	va_list args;
	char buf[128];
	int len;

	va_start(args, fmt);
	len = vsprintf(buf, fmt, args);
	va_end(args);
	hpt_copy_mem_info(pinfo, buf, len);
	return len;
}

#define HPT_DO_IOCTL(code, inbuf, insize, outbuf, outsize) ({\
	IOCTL_ARG arg;\
	arg.dwIoControlCode = code;\
	arg.lpInBuffer = inbuf;\
	arg.lpOutBuffer = outbuf;\
	arg.nInBufferSize = insize;\
	arg.nOutBufferSize = outsize;\
	arg.lpBytesReturned = 0;\
	hpt_do_ioctl(&arg);\
	arg.result;\
})

#define DEVICEID_VALID(id) ((id) && ((HPT_U32)(id)!=0xffffffff))

static int hpt_get_controller_count(void)
{
	HPT_U32 NumAdapters;

	if (HPT_DO_IOCTL(HPT_IOCTL_GET_CONTROLLER_COUNT,
		NULL, 0, &NumAdapters, sizeof(HPT_U32)))
		return -1;
	return NumAdapters;
}

static int hpt_get_controller_info_v2(int id, PCONTROLLER_INFO_V2 pInfo2)
{
	return HPT_DO_IOCTL(HPT_IOCTL_GET_CONTROLLER_INFO_V2,
				&id, sizeof(int), pInfo2, sizeof(CONTROLLER_INFO_V2));
}

static int hpt_get_physical_devices(DEVICEID * pIds, int nMaxCount)
{
	int i;
	HPT_U32 count = nMaxCount-1;

	if (HPT_DO_IOCTL(HPT_IOCTL_GET_PHYSICAL_DEVICES,
			&count, sizeof(HPT_U32), pIds, sizeof(DEVICEID)*nMaxCount))
		return -1;

	nMaxCount = (int)pIds[0];
	for (i=0; i<nMaxCount; i++) pIds[i] = pIds[i+1];
	return nMaxCount;

}
static int hpt_get_logical_devices(DEVICEID * pIds, int nMaxCount)
{
	int i;
	HPT_U32 count = nMaxCount-1;

	if (HPT_DO_IOCTL(HPT_IOCTL_GET_LOGICAL_DEVICES,
			&count, sizeof(HPT_U32), pIds, sizeof(DEVICEID)*nMaxCount))
		return -1;

	nMaxCount = (int)pIds[0];
	for (i=0; i<nMaxCount; i++) pIds[i] = pIds[i+1];
	return nMaxCount;
}

static int hpt_get_device_info_v3(DEVICEID id, PLOGICAL_DEVICE_INFO_V3 pInfo)
{
	return HPT_DO_IOCTL(HPT_IOCTL_GET_DEVICE_INFO_V3,
				&id, sizeof(DEVICEID), pInfo, sizeof(LOGICAL_DEVICE_INFO_V3));
}

static const char *get_array_status(PLOGICAL_DEVICE_INFO_V3 devinfo)
{
	static char s[64];

	if (devinfo->u.array.Flags & ARRAY_FLAG_DISABLED)
		return "Disabled";
	else if (devinfo->u.array.Flags & ARRAY_FLAG_TRANSFORMING)
		sprintf(s, "Expanding/Migrating %d.%d%%%s%s",
			devinfo->u.array.TransformingProgress/100, devinfo->u.array.TransformingProgress%100,
			(devinfo->u.array.Flags & (ARRAY_FLAG_NEEDBUILDING|ARRAY_FLAG_BROKEN))? ", Critical" : "",
			(devinfo->u.array.Flags & ARRAY_FLAG_NEEDINITIALIZING && !(devinfo->u.array.Flags & ARRAY_FLAG_REBUILDING) &&
			!(devinfo->u.array.Flags & ARRAY_FLAG_INITIALIZING))?", Unintialized":"");
	else if (devinfo->u.array.Flags & ARRAY_FLAG_BROKEN)
		return "Critical";
	else if (devinfo->u.array.Flags & ARRAY_FLAG_REBUILDING)
		sprintf(s,
			(devinfo->u.array.Flags & ARRAY_FLAG_NEEDINITIALIZING)?
				"Background initializing %d.%d%%" : "Rebuilding %d.%d%%",
			devinfo->u.array.RebuildingProgress/100, devinfo->u.array.RebuildingProgress%100);
	else if (devinfo->u.array.Flags & ARRAY_FLAG_VERIFYING)
		sprintf(s, "Verifying %d.%d%%",
			devinfo->u.array.RebuildingProgress/100, devinfo->u.array.RebuildingProgress%100);
	else if (devinfo->u.array.Flags & ARRAY_FLAG_INITIALIZING)
		sprintf(s, "Forground initializing %d.%d%%",
			devinfo->u.array.RebuildingProgress/100, devinfo->u.array.RebuildingProgress%100);
	else if (devinfo->u.array.Flags & ARRAY_FLAG_NEEDTRANSFORM)
		sprintf(s,"%s%s", "Need Expanding/Migrating",
			(devinfo->u.array.Flags & ARRAY_FLAG_NEEDINITIALIZING && !(devinfo->u.array.Flags & ARRAY_FLAG_REBUILDING) &&
			!(devinfo->u.array.Flags & ARRAY_FLAG_INITIALIZING))?", Unintialized":"");
	else if (devinfo->u.array.Flags & ARRAY_FLAG_NEEDINITIALIZING &&
		!(devinfo->u.array.Flags & ARRAY_FLAG_REBUILDING) &&
		!(devinfo->u.array.Flags & ARRAY_FLAG_INITIALIZING))
		sprintf(s,"%s", "Uninitialized");
	else if (devinfo->u.array.Flags & ARRAY_FLAG_NEEDBUILDING)
		return "Critical";
	else
		return "Normal";
	return s;
}

static void hpt_dump_devinfo(HPT_GET_INFO *pinfo, DEVICEID id, int indent)
{
	LOGICAL_DEVICE_INFO_V3 devinfo;
	int i;
	HPT_U8 sn[21];

	for (i=0; i<indent; i++) hpt_copy_info(pinfo, "    ");

	if (hpt_get_device_info_v3(id, &devinfo)) {
		hpt_copy_info(pinfo, "unknown\n");
		return;
	}

	switch (devinfo.Type) {
	case LDT_DEVICE:
		((char *)devinfo.u.device.IdentifyData.ModelNumber)[20] = 0;
		if (indent)
			if (devinfo.u.device.Flags & DEVICE_FLAG_DISABLED)
				hpt_copy_info(pinfo,"Missing\n");
			else
				hpt_copy_info(pinfo, "%d/%d/%d %s\n",
					devinfo.u.device.ControllerId+1,
					devinfo.u.device.PathId+1,
					devinfo.u.device.TargetId+1,
					devinfo.u.device.IdentifyData.ModelNumber
				);
		else {
			memcpy(sn, devinfo.u.device.IdentifyData.SerialNumber,
				sizeof(devinfo.u.device.IdentifyData.SerialNumber));
			ldm_ide_fixstring(sn, (sizeof(sn) & (~1)));
			sn[sizeof(sn) - 1] = 0;			
			hpt_copy_info(pinfo, "%d/%s%d/%d %s-%s, %dMB, %s %s%s%s%s\n",
				devinfo.u.device.ControllerId+1,
				(devinfo.u.device.Flags & DEVICE_FLAG_IN_ENCLOSURE) ? "E" : "",
				devinfo.u.device.PathId+1,
				devinfo.u.device.TargetId+1,
				devinfo.u.device.IdentifyData.ModelNumber, sn,
				(int)(devinfo.Capacity*512/1000000),
				(devinfo.u.device.Flags & DEVICE_FLAG_DISABLED)? "Disabled" : "Normal",
				devinfo.u.device.ReadAheadEnabled? "[RA]":"",
				devinfo.u.device.WriteCacheEnabled? "[WC]":"",
				devinfo.u.device.TCQEnabled? "[TCQ]":"",
				devinfo.u.device.NCQEnabled? "[NCQ]":""
			);
		}
		break;
	case LDT_ARRAY:
		if (devinfo.TargetId!=INVALID_TARGET_ID)
			hpt_copy_info(pinfo, "[DISK %d_%d] ", devinfo.VBusId, devinfo.TargetId);
		hpt_copy_info(pinfo, "%s (%s), %dMB, %s\n",
			devinfo.u.array.Name,
			devinfo.u.array.ArrayType==AT_RAID0? "RAID0" :
				devinfo.u.array.ArrayType==AT_RAID1? "RAID1" :
				devinfo.u.array.ArrayType==AT_RAID5? "RAID5" :
				devinfo.u.array.ArrayType==AT_RAID6? "RAID6" :
				devinfo.u.array.ArrayType==AT_JBOD? "JBOD" : "unknown",
			(int)(devinfo.Capacity*512/1000000),
			get_array_status(&devinfo)
		);
		for (i=0; i<devinfo.u.array.nDisk; i++) {
			if (DEVICEID_VALID(devinfo.u.array.Members[i])) {
				if ((1<<i) & devinfo.u.array.Critical_Members)
					hpt_copy_info(pinfo, "    *");
				hpt_dump_devinfo(pinfo, devinfo.u.array.Members[i], indent+1);
			}
			else
				hpt_copy_info(pinfo, "    Missing\n");
		}
		if (id==devinfo.u.array.TransformSource) {
			hpt_copy_info(pinfo, "    Expanding/Migrating to:\n");
			hpt_dump_devinfo(pinfo, devinfo.u.array.TransformTarget, indent+1);
		}
		break;
	}
}

#define MAX_PHYSICAL_DEVICE	128
int hpt_proc_get_info(struct Scsi_Host *host, char *buffer, char **start, off_t offset, int length)
{
	HPT_GET_INFO info;
	int i, j, count;
	CONTROLLER_INFO_V2 conInfo2;
	DEVICEID *ids;
	int devs;	
	PVBUS_EXT vbus_ext = get_vbus_ext(host);

	info.buffer     = buffer;
	info.buflength  = length;
	info.bufoffset  = offset;
	info.filpos     = 0;
	info.buffillen  = 0;

	if (start) *start = buffer;

	hpt_copy_info(&info, "%s %s\n\n", driver_name_long, driver_ver);

	count = hpt_get_controller_count();

	if (!(count>0) || !(ids = kmalloc(MAX_PHYSICAL_DEVICE * sizeof(DEVICEID), GFP_KERNEL)))
		return info.buffillen;

	if ((devs = hpt_get_physical_devices(ids, MAX_PHYSICAL_DEVICE))<0)
		return info.buffillen;
	
	for (j=0; j<count; j++) {
		int found;
		PHBA hba;
		if (hpt_get_controller_info_v2(j, &conInfo2))
			continue;

		found = 0;
		for (hba=vbus_ext->hba_list; hba; hba=hba->next) {
			if (conInfo2.pci_tree == hba->pciaddr.tree && conInfo2.pci_bus == hba->pciaddr.bus &&
					conInfo2.pci_device == hba->pciaddr.device && conInfo2.pci_function== hba->pciaddr.function)
				found = 1;
		}

		if (!found) continue;

		hpt_copy_info(&info, "Controller %d: %s\n", j+1, conInfo2.szProductID);
		hpt_copy_info(&info, "------------------------------------------------\n");

		for (i=0; i<devs; i++) {
			if (DEVICEID_VALID(ids[i])) {
				LOGICAL_DEVICE_INFO_V3 devinfo;
				if (hpt_get_device_info_v3(ids[i], &devinfo))
					continue;

				if (devinfo.VBusId == host->host_no)
					hpt_dump_devinfo(&info, ids[i], 0);
			}
		}
	}

	count = hpt_get_logical_devices(ids, MAX_PHYSICAL_DEVICE);
	hpt_copy_info(&info, "\nLogical devices\n");
	hpt_copy_info(&info, "------------------------------------------------\n");

	for (j=0; j<count; j++) {
		LOGICAL_DEVICE_INFO_V3 devinfo;
		if (hpt_get_device_info_v3(ids[j], &devinfo)) {
			hpt_copy_info(&info, "unknown\n");
			continue;
		}

		if (devinfo.VBusId == host->host_no)
			hpt_dump_devinfo(&info, ids[j], 0);
	}

	kfree(ids);

	return info.buffillen;
}

/* not belong to this file logically, but we want to use ioctl interface */
int __hpt_stop_tasks(PVBUS_EXT vbus_ext, DEVICEID id)
{
	LOGICAL_DEVICE_INFO_V3 devinfo;
	int i, result;
	DEVICEID param[2] = { id, 0 };

	if (hpt_get_device_info_v3(id, &devinfo))
		return -1;

	if (devinfo.Type!=LDT_ARRAY)
		return -1;

	if (devinfo.u.array.Flags & ARRAY_FLAG_REBUILDING)
		param[1] = AS_REBUILD_ABORT;
	else if (devinfo.u.array.Flags & ARRAY_FLAG_VERIFYING)
		param[1] = AS_VERIFY_ABORT;
	else if (devinfo.u.array.Flags & ARRAY_FLAG_INITIALIZING)
		param[1] = AS_INITIALIZE_ABORT;
	else if (devinfo.u.array.Flags & ARRAY_FLAG_TRANSFORMING)
		param[1] = AS_TRANSFORM_ABORT;
	else
		return -1;

	KdPrint(("SET_ARRAY_STATE(%x, %d)", param[0], param[1]));
	result = HPT_DO_IOCTL(HPT_IOCTL_SET_ARRAY_STATE,
				param, sizeof(param), 0, 0);

	for (i=0; i<devinfo.u.array.nDisk; i++)
		if (DEVICEID_VALID(devinfo.u.array.Members[i]))
			__hpt_stop_tasks(vbus_ext, devinfo.u.array.Members[i]);

	return result;
}

void hpt_stop_tasks(PVBUS_EXT vbus_ext)
{
	DEVICEID ids[32];
	int i, count;

	count = hpt_get_logical_devices((DEVICEID *)&ids, sizeof(ids)/sizeof(ids[0]));

	for (i=0; i<count; i++)
		__hpt_stop_tasks(vbus_ext, ids[i]);
}
