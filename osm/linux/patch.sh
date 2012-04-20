#!/bin/sh

if test "${KERNEL_VER-set}" = set; then echo "KERNEL_VER is not set"; exit 1; fi
if test "${KERNELDIR-set}" = set; then echo "KERNELDIR is not set"; exit 1; fi
if test "${HPT_ROOT-set}" = set; then echo "HPT_ROOT is not set"; exit 1; fi
if test "${TARGETNAME-set}" = set; then echo "TARGETNAME is not set"; exit 1; fi
if test "${TARGETMODS-set}" = set; then echo "TARGETMODS is not set"; exit 1; fi
if test "${TARGETOBJS-set}" = set; then echo "TARGETOBJS is not set"; exit 1; fi

#check to see if it is really a src dir
if [ ! -d ${KERNELDIR}/Documentation ] ; then
	echo "The target directory ${KERNELDIR} is not a full kernel source tree."
	exit 1
fi

ARCH=$( uname -m )
PRODUCTNAME=$( ( cd .. ; pwd ) | sed "s/\//\n/g" | tail -n1 )
UTARGETNAME=$( echo $TARGETNAME | tr "[:lower:]" "[:upper:]" )

echo "The target kernel version is ${KERNEL_VER}"

#echo "Make directory to hold the open source driver code"
rm -rf ${KERNELDIR}/drivers/scsi/${TARGETNAME}
mkdir ${KERNELDIR}/drivers/scsi/${TARGETNAME}
cp ${HPT_ROOT}/inc/*.h ${KERNELDIR}/drivers/scsi/${TARGETNAME}/
cp ${HPT_ROOT}/inc/linux/* ${KERNELDIR}/drivers/scsi/${TARGETNAME}/
cp ${HPT_ROOT}/osm/linux/* ${KERNELDIR}/drivers/scsi/${TARGETNAME}/
cd ${HPT_ROOT}/product
cp ${HPT_ROOT}/product/${PRODUCTNAME}/linux/* ${KERNELDIR}/drivers/scsi/${TARGETNAME}/

case "$KERNEL_VER" in
	2.4 )
	cd ${HPT_ROOT}/lib/linux/free-${ARCH}-regparm0
	ld -r -o ${ARCH}-${TARGETNAME}.obj ${TARGETMODS}
	mv ${ARCH}-${TARGETNAME}.obj $KERNELDIR/drivers/scsi/${TARGETNAME}/	
	cd $KERNELDIR/drivers/scsi/${TARGETNAME}
	if test ${ARCH} = "i686" ; then
		mv ${ARCH}-${TARGETNAME}.obj i386-${TARGETNAME}.obj
		ARCH=i386
	fi
	echo "#
# Default makefile for Linux $KERNEL_VER
# Copyright (C) 2004-2007 HighPoint Technologies, Inc. All Rights Reserved.
#

EXTRA_CFLAGS += ${C_DEFINE} -I\$(TOPDIR)/drivers/scsi

ifeq (\$(ARCH), x86_64)
EXTRA_CFLAGS += -mcmodel=kernel -DBITS_PER_LONG=64
endif

O_TARGET := ${TARGETNAME}.o

obj-m := \$(O_TARGET)
obj-y := ${TARGETOBJS} ${ARCH}-${TARGETNAME}.obj

include \$(TOPDIR)/Rules.make
" > Makefile
	rm -f Makefile.* *.sh *~ *.o *.ko

	grep "CONFIG_SCSI_${UTARGETNAME}" ../Config.in >> /dev/null
	if test $? -eq 1 ; then
		LINENUM=$(grep -n "SCSI\ low-level" ../Config.in | cut -f1 -d:)
		let LINENUM=$LINENUM+1
		mv ../Config.in ../Config.in.bak
		sed -e "
${LINENUM}a\\
dep_tristate \'HighPoint RocketRAID ${PRODUCTNAME} support\' CONFIG_SCSI_${UTARGETNAME} \$CONFIG_SCSI \$CONFIG_PCI" ../Config.in.bak > ../Config.in
	fi
	grep "CONFIG_SCSI_${UTARGETNAME}" ../Makefile >> /dev/null
	if test $? -eq 1 ; then
		LINENUM=$(grep -n "obj-\$(CONFIG_SCSI)" ../Makefile | cut -f1 -d:)
		let LINENUM=$LINENUM-2
		let LINENUM2=$LINENUM+3
		mv ../Makefile ../Makefile.bak
		sed -e "
${LINENUM}a\\
subdir-\$(CONFIG_SCSI_${UTARGETNAME})	+= ${TARGETNAME}
${LINENUM2}a\\
ifeq (\$(CONFIG_SCSI_${UTARGETNAME}),y)
${LINENUM2}a\\
  obj-\$(CONFIG_SCSI_${UTARGETNAME})	+= ${TARGETNAME}/${TARGETNAME}.o
${LINENUM2}a\\
endif
" ../Makefile.bak > ../Makefile
	fi
	grep "CONFIG_SCSI_${UTARGETNAME}" ../../../Documentation/Configure.help >> /dev/null
	if test $? -eq 1 ; then
		LINENUM=$(grep -n "#\ A\ couple\ of" ../../../Documentation/Configure.help | cut -f1 -d:)
		let LINENUM=$LINENUM-2
		mv ../../../Documentation/Configure.help ../../../Documentation/Configure.help.bak
		sed "
${LINENUM}a\\

${LINENUM}a\\
HPT RockedRAID ${PRODUCTNAME} support
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}
${LINENUM}a\\
  This is support for HighPoint ${PRODUCTNAME}
${LINENUM}a\\
  Host Adapter
${LINENUM}a\\

${LINENUM}a\\
  This also could be compiled as module called ${TARGETNAME}.o.
" ../../../Documentation/Configure.help.bak > ../../../Documentation/Configure.help
	fi
	if [ -d ../../../arch/i386 ];then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../arch/i386/defconfig >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../arch/i386/defconfig | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../arch/i386/defconfig ../../../arch/i386/defconfig.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../arch/i386/defconfig.bak > ../../../arch/i386/defconfig
		fi
	fi
	if [ -d ../../../arch/x86_64 ]; then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../arch/x86_64/defconfig >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../arch/i386/defconfig | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../arch/x86_64/defconfig ../../../arch/x86_64/defconfig.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../arch/x86_64/defconfig.bak > ../../../arch/x86_64/defconfig
		fi
	fi
	if [ -f ../../../.config ]; then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../.config >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../.config | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../.config ../../../.config.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../.config.bak > ../../../.config
		fi
	fi
	;;
	2.6 )
	cd ${HPT_ROOT}/lib/linux/free-${ARCH}-regparm0
	ld -r -o ${ARCH}-${TARGETNAME}.obj ${TARGETMODS}
	mv ${ARCH}-${TARGETNAME}.obj $KERNELDIR/drivers/scsi/${TARGETNAME}/	
	cd ${HPT_ROOT}/lib/linux/free-${ARCH}-regparm3
	ld -r -o ${ARCH}-${TARGETNAME}-3.obj ${TARGETMODS}
	mv ${ARCH}-${TARGETNAME}-3.obj $KERNELDIR/drivers/scsi/${TARGETNAME}/	
	cd $KERNELDIR/drivers/scsi/${TARGETNAME}
	if test ${ARCH} = "i686" ; then
		mv ${ARCH}-${TARGETNAME}.obj i386-${TARGETNAME}.obj
		mv ${ARCH}-${TARGETNAME}-3.obj i386-${TARGETNAME}-3.obj
		ARCH=i386
	fi
	echo "#
# Default makefile for Linux $KERNEL_VER
# Copyright (C) 2004-2007 HighPoint Technologies, Inc. All Rights Reserved.
#

EXTRA_CFLAGS += ${C_DEFINE} -I\$(TOPDIR)/drivers/scsi -I.

ifeq (\$(ARCH), x86_64)
EXTRA_CFLAGS += -mcmodel=kernel -DBITS_PER_LONG=64
endif

HPT_REGPARM := \$(shell echo \"\$(CFLAGS)\" | awk ' { match(\$\$0,\"-mregparm=\"); if (RSTART>0) print substr(\$\$0,RSTART+10,1);}')
HPT_REGPARM := \$(if \$(HPT_REGPARM),\$(HPT_REGPARM),0)

ifeq (\$(HPT_REGPARM), 0)
TARGETMOD := ${ARCH}-${TARGETNAME}
else
TARGETMOD := ${ARCH}-${TARGETNAME}-3
endif

obj-\$(CONFIG_SCSI_${UTARGETNAME}) := ${TARGETNAME}.o

${TARGETNAME}-objs := ${TARGETOBJS} \$(TARGETMOD).obj
" > Makefile
	rm -f Makefile.* *.sh *~ *.o *.ko
	grep -n "SCSI_${UTARGETNAME}" ../Kconfig >> /dev/null
	if test $? -eq 1 ; then
		mv ../Kconfig ../Kconfig.bak
		LINENUM=$(grep -n "SCSI\ low-level" ../Kconfig.bak | cut -f1 -d:)
		let LINENUM=$LINENUM+1
		sed "
${LINENUM}a\\

${LINENUM}a\\
config SCSI_${UTARGETNAME}
${LINENUM}a\\
	tristate \"HighPoint RocketRAID ${PRODUCTNAME} support\"
${LINENUM}a\\
	depends on PCI && SCSI
${LINENUM}a\\
	help
${LINENUM}a\\
	This is support for HighPoint ${PRODUCTNAME}
${LINENUM}a\\
	Host Adapter
${LINENUM}a\\

${LINENUM}a\\
	You can compile it as a module called ${TARGETNAME}.
" ../Kconfig.bak > ../Kconfig
	fi
	grep -n "CONFIG_SCSI_${UTARGETNAME}" ../Makefile >> /dev/null
	if test $? -eq 1 ; then
		mv ../Makefile ../Makefile.bak
		LINENUM=$(grep -n "obj-\$(CONFIG_SCSI_FC_ATTRS)" ../Makefile.bak | cut -f1 -d:)
		let LINENUM=$LINENUM+2
		sed "
${LINENUM}a\\
obj-\$(CONFIG_SCSI_${UTARGETNAME})  += ${TARGETNAME}/
" ../Makefile.bak > ../Makefile
	fi
	if [ -d ../../../arch/i386 ];then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../arch/i386/defconfig >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../arch/i386/defconfig | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../arch/i386/defconfig ../../../arch/i386/defconfig.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../arch/i386/defconfig.bak > ../../../arch/i386/defconfig
		fi
	fi
	if [ -d ../../arch/x86_64 ];then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../arch/x86_64/defconfig >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../arch/x86_64/defconfig | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../arch/x86_64/defconfig ../../../arch/x86_64/defconfig.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../arch/x86_64/defconfig.bak > ../../../arch/x86_64/defconfig
		fi
	fi
	if [ -f ../../../.config ]; then
		grep "CONFIG_SCSI_${UTARGETNAME}" ../../../.config >> /dev/null
		if test $? -eq 1 ; then
			LINENUM=$(grep -n "#\ SCSI\ low-level" ../../../.config | cut -f1 -d:)
			let LINENUM=$LINENUM+1
			mv ../../../.config ../../../.config.bak
			sed "
${LINENUM}a\\
CONFIG_SCSI_${UTARGETNAME}=y
" ../../../.config.bak > ../../../.config
		fi
	fi
	;;
	* )
	echo " "
	echo "ERROR! Target kernel version is not supported."
	echo "Erasing what we have done..."
	rm -rf ${KERNRLDIR}/drivers/scsi/${TARGETNAME}
	exit 1
esac
echo "Kernel source has been patched successfully."
exit 0
