#!/bin/sh

if test "${KERNEL_VER-set}" = set; then echo "KERNEL_VER is not set"; exit 1; fi
if test "${TARGETNAME-set}" = set; then echo "TARGETNAME is not set"; exit 1; fi

PWD=`pwd`

case ${KERNEL_VER} in
	2.4 )
	OBJ=o
	MODVER=`modinfo -f%{kernel_version} ${PWD}/${TARGETNAME}.${OBJ}`
	;;
	2.6 | 3.* | 4.* )
	OBJ=ko
	MODVER=`modinfo -F vermagic ${PWD}/${TARGETNAME}.${OBJ} | cut -d' ' -f1`
	;;
esac

if test "${MODVER}" = "" ; then
	echo "Can not get kernel version from ${TARGETNAME}.${OBJ}."
	exit 1
fi

if test "${MODVER}" = "`uname -r`"; then
	echo "You made a module which is for current kernel ${MODVER}."
	echo "Deleting previous installed driver module ${TARGETNAME}..."
	HPTMV6=$( find /lib/modules/`uname -r`/* -name ${TARGETNAME}.${OBJ} )
	rm -f ${HPTMV6}
	rm -rf /lib/modules/`uname -r`/kernel/drivers/scsi/${TARGETNAME}
	mkdir -p /lib/modules/`uname -r`/kernel/drivers/scsi/${TARGETNAME}
	echo "Install the new driver module..."
	cp ${PWD}/${TARGETNAME}.${OBJ} /lib/modules/`uname -r`/kernel/drivers/scsi/${TARGETNAME}
	echo "Removing conflicted driver module..."
	if [ "${TARGETNAME}" = "rr272x_1x" -o "${TARGETNAME}" = "rr274x_3x" -o  "${TARGETNAME}" = "rr276x" -o "${TARGETNAME}" = "rr278x" ] ; then
		find /lib/modules/`uname -r`/kernel -name mvsas.* -exec rm -f {} \;
	elif [ "${TARGETNAME}" = "hptmv" -o "${TARGETNAME}" = "hptmv6" -o  "${TARGETNAME}" = "rr174x" -o "${TARGETNAME}" = "rr2310_00" -o "${TARGETNAME}" = "rr172x" ] ; then
		find /lib/modules/`uname -r`/kernel -name sata_mv.* -exec rm -f {} \;
	elif [ "$TARGETNAME" = "hpt374" ]; then
		find /lib/modules/`uname -r`/kernel -name hpt366.* -exec rm -f {} \;
		find /lib/modules/`uname -r`/kernel -name pata_hpt37x.* -exec rm -f {} \;
	fi
	echo -n "Updating module dependencies..."
	depmod -a
	echo "Done."
elif [ -d /lib/modules/${MODVER} ]; then
	echo "You made a module for ${MODVER} which does not match current kernel."
	echo "The driver will be installed for kernel ${MODVER}."
	echo "Deleting previous installed driver module ${TARGETNAME}..."
	if [ "${TARGETNAME}" = "rr272x_1x" -o "${TARGETNAME}" = "rr274x_3x" -o  "${TARGETNAME}" = "rr276x" -o "${TARGETNAME}" = "rr278x" ] ; then
		find /lib/modules/${MODVER}/kernel -name mvsas.* -exec rm -f {} \;
	fi
	HPTMV6=$( find /lib/modules/${MODVER}/* -name ${TARGETNAME}.${OBJ} )
	rm -f ${HPTMV6}
	rm -rf /lib/modules/${MODVER}/kernel/drivers/scsi/${TARGETNAME}
	mkdir -p /lib/modules/${MODVER}/kernel/drivers/scsi/${TARGETNAME}
	echo "Install the new driver module..."
	cp -f ${PWD}/${TARGETNAME}.${OBJ} /lib/modules/${MODVER}/kernel/drivers/scsi/${TARGETNAME}
	echo "Removing conflicted driver module..."
	if [ "${TARGETNAME}" = "hptmv" -o "${TARGETNAME}" = "hptmv6" ] ; then
		find /lib/modules/${MODVER}/kernel -name sata_mv.* -exec rm -f {} \;
	elif [ "$TARGETNAME" = "hpt374" ]; then
		find /lib/modules/${MODVER}/kernel -name hpt366.* -exec rm -f {} \;
		find /lib/modules/${MODVER}/kernel -name pata_hpt37x.* -exec rm -f {} \;
	fi
	echo -n "Updating module dependencies..."
	depmod -a ${MODVER}
	echo "Done."
else
	echo "The driver is built for kernel ${MODVER} and can not be installed to current system."
	exit 1
fi

MKINITRD=`which mkinitrd 2> /dev/null`
[ "$MKINITRD" = "" ] && MKINITRD=`which mkinitramfs 2> /dev/null`
[ "$MKINITRD" = "" ] && MKINITRD=`which dracut 2> /dev/null`

if test "$MKINITRD" = "" ; then
	echo "Can not find command 'mkinitrd' or 'mkinitramfs' or 'dracut'."
	exit 1
fi

echo "Checking for initrd images to be updated..."

#### Next if the system is on controller, make the system loadable
# mandrake redhat fedora debian rhel turbo sles redflag
distinfo() {
	local issue=/etc/issue
	[ -e /etc/mandrake-release ] && { echo mandrake; return; }
	[ -e /etc/redhat-release ] && { echo redhat; return; }
	[ -e /etc/fedora-release ] && { echo fedora; return; }
	[ -e /etc/SuSE-release ] && { echo suse; return; }
	[ -e /etc/debian_version ] && { echo debian; return; }
	[ -e /etc/turbolinux-release ] && { echo turbo; return; }
	if [ -e $issue ]; then
		grep -i "debian" $issue && { echo debian; return; }
		grep -i "ubuntu" $issue && { echo ubuntu; return; }
		grep -i "red *hat" $issue && { echo redhat; return; }
		grep -i "suse\|s\.u\.s\.e" $issue && { echo suse; return; }
	fi
}

DIST=`distinfo`
case ${DIST} in
fedora | redhat )
	if [ -f /boot/initrd-${MODVER}.img ] ; then
		if [ ! -f /boot/initrd-${MODVER}.img.${TARGETNAME} ]; then
			echo "Backup /boot/initrd-${MODVER}.img to /boot/initrd-${MODVER}.img.${TARGETNAME}."
			mv /boot/initrd-${MODVER}.img /boot/initrd-${MODVER}.img.${TARGETNAME}
		else
			rm /boot/initrd-${MODVER}.img
		fi
		$MKINITRD /boot/initrd-${MODVER}.img ${MODVER}
	elif [ -f /boot/initramfs-${MODVER}.img ] ; then
		if [ ! -f /boot/initramfs-${MODVER}.img.${TARGETNAME} ]; then
			echo "Backup /boot/initramfs-${MODVER}.img to /boot/initramfs-${MODVER}.img.${TARGETNAME}."
			mv /boot/initramfs-${MODVER}.img /boot/initramfs-${MODVER}.img.${TARGETNAME}
		else
			rm /boot/initramfs-${MODVER}.img
		fi
		$MKINITRD /boot/initramfs-${MODVER}.img ${MODVER}
	else
		echo "WARNING!!! No initrd image found, skip mkinitrd."
		sleep 3
	fi
;;
suse )
	if [ -f /etc/modprobe.d/unsupported-modules ]; then
		echo "Enable loading of unsupported modules."
		sed -i /^allow_unsupported_modules/s/0/1/g /etc/modprobe.d/unsupported-modules
	fi
	if [ -f /etc/sysconfig/hardware/config ]; then
		echo "Enable loading of unsupported modules."
		sed -i /^LOAD_UNSUPPORTED_MODULES_AUTOMATICALLY/s/no/yes/g /etc/sysconfig/hardware/config
	fi
	if [ -f /boot/initrd-${MODVER} ] ; then
		if [ ! -f /boot/initrd-${MODVER}.${TARGETNAME} ]; then
			echo "Backup /boot/initrd-${MODVER} to /boot/initrd-${MODVER}.${TARGETNAME}."
			mv /boot/initrd-${MODVER} /boot/initrd-${MODVER}.${TARGETNAME}
		fi
	fi
	$MKINITRD
;;
debian | ubuntu )
	if [ -f /boot/initrd.img-${MODVER} ] ; then
		if [ ! -f /boot/initrd.img-${MODVER}.${TARGETNAME} ]; then
			echo "backup /boot/initrd.img-${MODVER} to /boot/initrd.img-${MODVER}.${TARGETNAME}."
			mv /boot/initrd.img-${MODVER} /boot/initrd.img-${MODVER}.${TARGETNAME}
		else
			rm /boot/initrd.img-${MODVER}
		fi
	fi
	if [ -f /etc/initramfs-tools/initramfs.conf ]; then
		if grep ^ROOTDELAY -s -q /etc/initramfs-tools/initramfs.conf; then
			sed -i s"#^ROOTDELAY.*#ROOTDELAY=180#" /etc/initramfs-tools/initramfs.conf
		else
			echo "ROOTDELAY=180" >> /etc/initramfs-tools/initramfs.conf
		fi
	fi
	$MKINITRD -o /boot/initrd.img-${MODVER} ${MODVER}
;;
* )
	echo "Unknown distribution - You have to update the initrd image manually."
;;
esac
