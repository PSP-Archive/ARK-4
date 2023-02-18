#ifndef __PSPUSBDEVICE_H__
#define __PSPUSBDEVICE_H__

/**
 * This functions require flash0:/kd/_usbdevice.prx to be loaded/started first.

 * Link with pspusbdevice for user mode access or with pspusbdevice_driver for kernel access
*/

#define PSP_USBDEVICE_FLASH0	0
#define	PSP_USBDEVICE_FLASH1	1
#define PSP_USBDEVICE_FLASH2	2
#define PSP_USBDEVICE_FLASH3	3
#define PSP_USBDEVICE_UMD9660	4

#define UNASSIGN_MASK_FLASH0	1
#define UNASSIGN_MASK_FLASH1	2
#define UNASSIGN_MASK_FLASH2	3
#define UNASSIGN_MASK_FLASH3	4

/**
 * Sets the usb device. Call this function when you are about to do the sceUsbStart and sceUsbActivate stuff
 *
 * @param device - The usb device, one of listed above.
 * @param ronly - If  non-zero indicates read only access. This parameters is ignored for PSP_USBDEVICE_UMD9660
 * @param unassign_mask - It unassigns automatically the flashes indicated by the mask. 
 * The flashes will be reassigned automatically after calling pspUsbDeviceFinishDevice
 *
 * Set this param to 0 if you don't need it (vshctrl doesn't use it).
 *
 * @returns 0 on success, < 0 on error.
*/
int pspUsbDeviceSetDevice(u32 device, int ronly, int unassign_mask);

/**
 * Finishes the usb device. Call this function after stoping usbstor driver
 *
 * @returns 0 in success, < 0 on error
*/
int pspUsbDeviceFinishDevice();


#endif

