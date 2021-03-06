#ifndef __USBDEV_H_INCLUDE__
#define __USBDEV_H_INCLUDE__

#include <host/wdm.h>

#define USBD_IOCTL_INDEX	0x0001
#define USBDIORESETEP		(USBD_IOCTL_INDEX+0)
#define USBDIORESETIF		(USBD_IOCTL_INDEX+1)
#define USBDIOCYCLEIF		(USBD_IOCTL_INDEX+2)
#define USBDIOGETIFDESC		(USBD_IOCTL_INDEX+3)
#define USBDIOGETSTRING		(USBD_IOCTL_INDEX+4)

#define USBD_IOCTL_CODE(code, dir, method)			\
	IOCTL_CODE(code, dir, method)
#define IOCTL_USBD_RESETEP					\
	USBD_IOCTL_CODE(USBDIORESETEP,				\
			IOCTL_INPUT, METHOD_BUFFERED)
#define IOCTL_USBD_RESETIF					\
	USBD_IOCTL_CODE(USBDIORESETIF,				\
			IOCTL_INPUT, METHOD_BUFFERED)
#define IOCTL_USBD_CYCLEIF					\
	USBD_IOCTL_CODE(USBDIOCYCLEIF,				\
			IOCTL_INPUT, METHOD_BUFFERED)
#define IOCTL_USBD_GETIFDESC					\
	USBD_IOCTL_CODE(USBDIOGETIFDESC,			\
			IOCTL_OUTPUT, METHOD_BUFFERED)
#define IOCTL_USBD_GETSTRING					\
	USBD_IOCTL_CODE(USBDIOGETSTRING,			\
			IOCTL_OUTPUT, METHOD_BUFFERED)

#endif /* __USBDEV_H_INCLUDE__ */
