#include <serial/serial.h>
#include <logging.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>
#include <fcntl.h>
#include <sys/termios.h>


/* Create an IOKit iterator for all matching serial devices. */
static kern_return_t create_serial_iterator(io_iterator_t *serial_iterator)
{
	kern_return_t kresult;
	mach_port_t masterport;
	CFMutableDictionaryRef matchclasses;

	if((kresult = IOMasterPort(MACH_PORT_NULL, &masterport))
			!= KERN_SUCCESS) {
		logerror("IOMasterPort returned %d\n", kresult);
		return kresult;
	}

	if((matchclasses = IOServiceMatching(kIOSerialBSDServiceValue))
			== NULL) {
		logerror("IOServiceMatching returned NULL\n");
		return kIOReturnNoDevice;
	}

	CFDictionarySetValue(matchclasses, CFSTR(kIOSerialBSDTypeKey),
			CFSTR(kIOSerialBSDRS232Type));

	if((kresult = IOServiceGetMatchingServices(masterport,
					matchclasses, serial_iterator))
			!= KERN_SUCCESS) {
		logerror("IOServiceGetMatchingServices returned %d\n",
				kresult);
	}

	return kresult;
}

static char *get_registry_string(io_object_t sObj, const char *prop_name)
{
	static char resstr[256];
	CFTypeRef nameCFstring;
	CFTypeRef propCFstring = CFStringCreateWithCString(kCFAllocatorDefault,
			prop_name, kCFStringEncodingASCII);

	resstr[0] = 0;
	nameCFstring = IORegistryEntryCreateCFProperty(sObj, propCFstring,
			kCFAllocatorDefault, 0);
	if(nameCFstring) {
		CFStringGetCString(nameCFstring, resstr, sizeof(resstr),
				kCFStringEncodingASCII);
		CFRelease(nameCFstring);
	}
	CFRelease(propCFstring);
	return resstr;
}

void serial_free_device_list(char **device_paths)
{
	size_t i;

	for(i = 0; device_paths[i]; i++) {
		free(device_paths[i]);
	}
	free(device_paths);
}

char **serial_device_list(void)
{
	char **list;
	size_t count = 0;
	size_t i;
	io_iterator_t serial_iterator;
	io_object_t serial_object;

	if(create_serial_iterator(&serial_iterator) != KERN_SUCCESS) {
		logerror("could not create serial iterator\n");
		return NULL;
	}

	/* Count services */
	while((serial_object = IOIteratorNext(serial_iterator)) != 0) {
		count++;
	}
	IOIteratorReset(serial_iterator);

	/* Allocate the device list */
	if((list = malloc(sizeof(char *) * (count + 1))) == NULL) {
		logerror("could not allocate device list: %s\n",
				strerror(errno));
		goto exit;
	}
	memset(list, 0, sizeof(char *) * (count + 1));

	/* Fill in the device list */
	i = 0;
	while(((serial_object = IOIteratorNext(serial_iterator)) != 0)
			&& (i < count)) {
		char *ttystr = get_registry_string(serial_object,
				kIOCalloutDeviceKey);

		if((list[i] = malloc(strlen(ttystr) + 1)) == NULL) {
			logerror("could not allocate device string: %s\n",
					strerror(errno));
			serial_free_device_list(list);
			list = NULL;
			goto exit;
		}

		strcpy(list[i], ttystr);
		i++;
	}

exit:
	IOObjectRelease(serial_iterator);
	return list;
}

struct serial_device *serial_open(const char *device_path, uint32_t baud,
		uint8_t stopbits)
{
	struct termios options;
	struct serial_device *dev;

	if((dev = malloc(sizeof(*dev))) == NULL) {
		logerror("could not allocate device structure: %s\n",
				strerror(errno));
		return NULL;
	}

	if((dev->fd = open(device_path, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
		logerror("could not open device: %s\n", strerror(errno));
		goto error_exit;
	}

	if(fcntl(dev->fd, F_SETFL, 0) == -1) {
		logerror("could not fcntl: %s\n", strerror(errno));
		goto error_exit;
	}

	if(tcgetattr(dev->fd, &dev->saved_tty_attrs) == -1) {
		logerror("could not save attributes: %s\n", strerror(errno));
		goto error_exit;
	}

	/* Set raw input, one tenth second timeout. These options are
	 * documented in the man page for termios.
	 */
	memcpy(&options, &dev->saved_tty_attrs, sizeof(options));
	/* Character size */
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	/* Parity disable */
	options.c_cflag &= ~PARENB;
	/* Hardware flow control disable */
	options.c_cflag |= CLOCAL;
	/* Receiver enable */
	options.c_cflag |= CREAD;
	/* Stop Bits */
	if(stopbits == 1) {
		options.c_cflag &= ~CSTOPB;
	} else if(stopbits == 2) {
		options.c_cflag |= CSTOPB;
	} else {
		logerror("invalid stop bits specified\n");
		goto error_exit;
	}
	/* Disable canonicalize, echo, and signals */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	/* Disable output processing */
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 1;

	if(baud != 0) {
		options.c_ispeed = baud;
		options.c_ospeed = baud;
	}

	/* Set the attributes */
	if(tcsetattr(dev->fd, TCSANOW, &options) == -1) {
		logerror("could not set attributes: %s\n", strerror(errno));
		goto error_exit;
	}

	return dev;

error_exit:
	if(dev->fd >= 0)
		close(dev->fd);
	free(dev);
	return NULL;
}

void serial_close(struct serial_device *dev)
{
	logverbose("fd:%d\n", dev->fd);

	if(dev->fd >= 0) {
		tcsetattr(dev->fd, TCSANOW, &dev->saved_tty_attrs);
		close(dev->fd);
	}

	free(dev);
}

ssize_t serial_write(struct serial_device *dev, const void *buf, size_t len)
{
	ssize_t ret = len;
	ssize_t written = 0;

	while(len) {
		if((written = write(dev->fd, buf, len)) == -1) {
			logerror("write error: %s\n", strerror(errno));
			ret = -1;
			break;
		}
		len -= written;
		buf = (void *)((char *)buf + written);
	}

	return ret;
}

ssize_t serial_read(struct serial_device *dev, void *buf, size_t len)
{
	ssize_t bread;

	if(buf == NULL || len == 0) {
		return -EINVAL;
	}

	if((bread = read(dev->fd, buf, len)) < 0) {
		if(errno == EAGAIN) {
			bread = 0;
		} else {
			logerror("read error: %s\n", strerror(errno));
		}
	}

	return bread;
}

void serial_flush(struct serial_device *dev)
{
	uint8_t buf;
	while(serial_read(dev, &buf, 1));
}
