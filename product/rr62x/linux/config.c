/****************************************************************************
 * config.c - auto-generated file
 ****************************************************************************/
#include "osm_linux.h"

#define INIT_MODULE(mod) \
	do {\
		extern int init_module_##mod(void);\
		init_module_##mod();\
	} while(0)

int init_config(void)
{
	INIT_MODULE(him_magni);
	INIT_MODULE(vdev_raw);
	INIT_MODULE(partition);
	INIT_MODULE(raid0);
	INIT_MODULE(raid1);
	INIT_MODULE(raid5);
	INIT_MODULE(jbod);
	return 0;
}

char driver_name[] = "rr62x";
char driver_name_long[] = "RocketRAID 62x SATA controller driver";
char driver_ver[] = "v1.2 (" __DATE__ " " __TIME__ ")";
int  osm_max_targets = 32;

int os_max_cache_size = 0x800000;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static const struct pci_device_id hpt_pci_tbl[] = {
	{PCI_DEVICE(0x1103, 0x0620), 0, 0, 0},
	{PCI_DEVICE(0x1103, 0x0622), 0, 0, 0},
	{}
};
MODULE_DEVICE_TABLE(pci, hpt_pci_tbl);
#endif
