/*
 * $Id: osm.h,v 1.21 2008/03/21 05:42:39 gmm Exp $
 * Copyright (C) 2004-2005 HighPoint Technologies, Inc. All rights reserved.
 */
#ifndef _HPT_OSM_H_
#define _HPT_OSM_H_

#define VERMAGIC_OSM 19

#define os_max_queue_comm 64
#define os_max_sg_descriptors 18


extern int os_max_cache_size;


#define DMAPOOL_PAGE_SIZE 0x1000 /* linux PAGE_SIZE (i386/x86_64) */
#define os_max_cache_pages (os_max_cache_size/DMAPOOL_PAGE_SIZE)

/* data types */
typedef unsigned int HPT_UINT, HPT_U32;
typedef unsigned long HPT_UPTR;
typedef unsigned short HPT_U16;
typedef unsigned char HPT_U8;
typedef unsigned long HPT_TIME;
typedef unsigned long long HPT_U64;

HPT_U64 CPU_TO_LE64(HPT_U64 x);
HPT_U32 CPU_TO_LE32(HPT_U32 x);
HPT_U16 CPU_TO_LE16(HPT_U16 x);
HPT_U64 LE64_TO_CPU(HPT_U64 x);
HPT_U32 LE32_TO_CPU(HPT_U32 x);
HPT_U16 LE16_TO_CPU(HPT_U16 x);
HPT_U64 CPU_TO_BE64(HPT_U64 x);
HPT_U32 CPU_TO_BE32(HPT_U32 x);
HPT_U16 CPU_TO_BE16(HPT_U16 x);
HPT_U64 BE64_TO_CPU(HPT_U64 x);
HPT_U32 BE32_TO_CPU(HPT_U32 x);
HPT_U16 BE16_TO_CPU(HPT_U16 x);

#define FAR
#define EXTERN_C
#define HPT_INLINE inline

typedef void * HPT_PTR;

typedef HPT_U64 HPT_LBA;
typedef HPT_U64 HPT_RAW_LBA;
#define MAX_LBA_VALUE 0xffffffffffffffffull
#define MAX_RAW_LBA_VALUE MAX_LBA_VALUE
#define RAW_LBA(x) (x)
#define LO_LBA(x) ((HPT_U32)(x))
#define HI_LBA(x) (sizeof(HPT_LBA)>4? (HPT_U32)((x)>>32) : 0)
#define LBA_FORMAT_STR "0x%llX"

typedef HPT_U64 BUS_ADDRESS;
#define LO_BUSADDR(x) ((HPT_U32)(x))
#define HI_BUSADDR(x) (sizeof(BUS_ADDRESS)>4? (x)>>32 : 0)

typedef unsigned char HPT_BOOL;
#define HPT_TRUE  1
#define HPT_FALSE 0

typedef struct _TIME_RECORD {
   HPT_U32        seconds:6;      /* 0 - 59 */
   HPT_U32        minutes:6;      /* 0 - 59 */
   HPT_U32        month:4;        /* 1 - 12 */
   HPT_U32        hours:6;        /* 0 - 59 */
   HPT_U32        day:5;          /* 1 - 31 */
   HPT_U32        year:5;         /* 0=2000, 31=2031 */
} TIME_RECORD;

/* hardware access */
HPT_U8   os_inb  (void *port);
HPT_U16  os_inw  (void *port);
HPT_U32  os_inl  (void *port);
void     os_outb (void *port, HPT_U8 value);
void     os_outw (void *port, HPT_U16 value);
void     os_outl (void *port, HPT_U32 value);
void     os_insw (void *port, HPT_U16 *buffer, HPT_U32 count);
void     os_outsw(void *port, HPT_U16 *buffer, HPT_U32 count);

HPT_U8   os_readb  (void *addr);
HPT_U16  os_readw  (void *addr);
HPT_U32  os_readl  (void *addr);
void     os_writeb (void *addr, HPT_U8 value);
void     os_writew (void *addr, HPT_U16 value);
void     os_writel (void *addr, HPT_U32 value);

/* PCI configuration space for specified device*/
HPT_U8   os_pci_readb (void *osext, HPT_U8 offset);
HPT_U16  os_pci_readw (void *osext, HPT_U8 offset);
HPT_U32  os_pci_readl (void *osext, HPT_U8 offset);
void     os_pci_writeb(void *osext, HPT_U8 offset, HPT_U8 value);
void     os_pci_writew(void *osext, HPT_U8 offset, HPT_U16 value);
void     os_pci_writel(void *osext, HPT_U8 offset, HPT_U32 value);

/* obsolute interface */
#define MAX_PCI_BUS_NUMBER 0xff
#define MAX_PCI_DEVICE_NUMBER 32
#define MAX_PCI_FUNC_NUMBER 8
#define pcicfg_read_dword(bus, dev, fn, reg) 0xffff


void *os_map_pci_bar(
	void *osext, 
	int index,   
	HPT_U32 offset,
	HPT_U32 length
);


void os_unmap_pci_bar(void *osext, void *base);

struct _SG;
void *os_kmap_sgptr(struct _SG *psg);
void os_kunmap_sgptr(void *ptr);
#define os_set_sgptr(psg, ptr) (psg)->addr.bus = (BUS_ADDRESS)(HPT_UPTR)(ptr)

/* timer */
void *os_add_timer(void *osext, HPT_U32 microseconds, void (*proc)(void *), void *arg);
void  os_del_timer(void *handle);
void  os_request_timer(void * osext, HPT_U32 interval);
HPT_TIME os_query_time(void);

/* task */
#define OS_SUPPORT_TASK

typedef struct _OSM_TASK {
	struct _OSM_TASK *next;
	void (*func)(void *vbus, void *data);
	void *data;
}
OSM_TASK;

void os_schedule_task(void *osext, OSM_TASK *task);

/* misc */
HPT_U32 os_get_stamp(void);
void os_stallexec(HPT_U32 microseconds);

#ifndef _LINUX_STRING_H_
#define memcpy(dst, src, size) __builtin_memcpy((dst), (src), (size))
#define memcmp(dst, src, size) __builtin_memcmp((dst), (src), (size))
#define memset(dst, value, size) __builtin_memset((dst), (value), (size))
#define strcpy(dst, src) \
	if (__builtin_constant_p(src)) \
		__builtin_strcpy((dst), (src)); \
	else { \
		char * d = dst;\
		const char *s = src;\
		while ((*d++=*s++)!=0);\
	}
#endif

#define farMemoryCopy(a,b,c) memcpy((char *)(a), (char *)(b), (HPT_U32)c)



void os_register_device(void *osext, int target_id);
void os_unregister_device(void *osext, int target_id);
int os_query_remove_device(void *osext, int target_id);
int os_revalidate_device(void *osext, int target_id);

HPT_U8 os_get_vbus_seq(void *osext);

/* debug support */
int  os_printk(char *fmt, ...);

#if DBG
extern int hpt_dbg_level;
#define KdPrint(x)  do { if (hpt_dbg_level) os_printk x; } while (0)
void __os_dbgbreak(const char *file, int line);
#define os_dbgbreak() __os_dbgbreak(__FILE__, __LINE__)
#define HPT_ASSERT(x) do { if (!(x)) os_dbgbreak(); } while (0)
void os_check_stack(const char *location, int size);
#define HPT_CHECK_STACK(size) os_check_stack(__FUNCTION__, (size))
#else 
#define KdPrint(x)
#define HPT_ASSERT(x)
#define HPT_CHECK_STACK(size)
#endif

#define OsPrint(x) do { os_printk x; } while (0)

#endif
