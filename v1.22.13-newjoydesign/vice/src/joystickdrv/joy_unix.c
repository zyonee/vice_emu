#include <linux/joystick.h>
#include <sys/types.h>
#include <fcntl.h>

#include "joystick.h"
#include "log.h"
#include "lib.h"

typedef struct joy_unix_priv_s {
    char* devfile;
    int fd;
} joy_unix_priv_t;

log_t joystick_log = LOG_ERR;

static void new_joystick(int i, void* dev)
{
    struct js_event e;
    joy_unix_priv_t* priv = (joy_unix_priv_t*)dev;

	/* Read all queued events. */
    while(read(priv->fd, &e, sizeof(struct js_event))
/*    while (q*/
	       == sizeof(struct js_event))
	{
        switch (e.type & ~JS_EVENT_INIT) {
        case JS_EVENT_BUTTON:
            joystick_set_value_and(i, ~16); /* reset fire bit */
            if (e.value)
            joystick_set_value_or(i, 16);
            break;

        case JS_EVENT_AXIS:
            if (e.number == 0) {
                joystick_set_value_and(i, 19); /* reset 2 bit */
            if (e.value > 16384)
                joystick_set_value_or(i, 8);
            else if (e.value < -16384)
                joystick_set_value_or(i, 4);
            }
            if (e.number == 1) {
               joystick_set_value_and(i, 28); /* reset 2 bit */
                if (e.value > 16384)
                    joystick_set_value_or(i, 2);
                else if (e.value < -16384)
                    joystick_set_value_or(i, 1);
            }
            break;
        }
    }
}

static int new_joystick_open(void* dev)
{
    joy_unix_priv_t* priv = (joy_unix_priv_t*)dev;
    priv->fd = open(priv->devfile, O_RDONLY);
    if (priv->fd == -1) {
        log_message(joystick_log, _("Failed opening of %s"), priv->devfile);
        return 0;
    }
    log_message(joystick_log, _("Opening of %s"), priv->devfile);
    fcntl(priv->fd, F_SETFL, O_NONBLOCK);
    return 1;
}

static void new_joystick_close(void* dev)
{
    joy_unix_priv_t* priv = (joy_unix_priv_t*)dev;
    close(priv->fd);
}

static void new_joystick_destroy(void* dev)
{
    joy_unix_priv_t* priv = (joy_unix_priv_t*)dev;
    lib_free(priv->devfile);
}

static joy_device_t unix_joystick_device = {
    new_joystick,
    new_joystick_open,
    new_joystick_close,
    new_joystick_destroy
} ;

static enum {
    JOY_INIT_OK,
    JOY_INIT_FILE_NOT_FOUND,
    JOY_INIT_READ_ERROR,
    JOY_INIT_IOCTL_FAILED
} joy_init_one (const char* devname, int port)
{
    joy_data_t* dev;
    struct JS_DATA_TYPE js;
    joy_unix_priv_t* priv;
    int  ajoyfd = open(devname, O_RDONLY);
    int ver = 0;
    int axes, buttons;
    char name[60];

    if (ajoyfd < 0)
        return JOY_INIT_FILE_NOT_FOUND;

    if (read (ajoyfd, &js, sizeof(struct JS_DATA_TYPE)) < 0) {
        close (ajoyfd);
        return JOY_INIT_READ_ERROR;
    }
    if (ioctl(ajoyfd, JSIOCGVERSION, &ver)) {
        close (ajoyfd);
        log_message(joystick_log, _("%s unknown type"), dev);
        log_message(joystick_log,
                            _("Built in driver version: %d.%d.%d"),
                            JS_VERSION >> 16, (JS_VERSION >> 8) & 0xff,
                            JS_VERSION & 0xff);
        log_message(joystick_log, _("Kernel driver version  : 0.8 ??"));
        log_message(joystick_log,
                            _("Please update your Joystick driver !"));
        log_message(joystick_log, _("Fall back to old api routine"));
        return JOY_INIT_IOCTL_FAILED;
    }
    ioctl(ajoyfd, JSIOCGVERSION, &ver);
    ioctl(ajoyfd, JSIOCGAXES, &axes);
    ioctl(ajoyfd, JSIOCGBUTTONS, &buttons);
    ioctl(ajoyfd, JSIOCGNAME (sizeof (name)), name);
            log_message(joystick_log, "%s is %s", dev, name);
            log_message(joystick_log, _("Built in driver version: %d.%d.%d"),
                        JS_VERSION >> 16, (JS_VERSION >> 8) & 0xff,
                        JS_VERSION & 0xff);
            log_message(joystick_log, _("Kernel driver version  : %d.%d.%d"),
                      ver >> 16, (ver >> 8) & 0xff, ver & 0xff);
    
    dev = lib_malloc(sizeof(joy_device_t));
    priv = lib_malloc(sizeof(joy_unix_priv_t));
    priv->devfile = lib_msprintf("%s", devname);;
    dev->name = lib_msprintf("%s", name);
    dev->priv = priv;
    dev->device = &unix_joystick_device;
    joystick_register_device(dev);
        
    return JOY_INIT_OK;
}

int joy_unix_init()
{
    int i;
    if (joystick_log == LOG_ERR)
        joystick_log = log_open("Joystick");

    log_message(joystick_log, _("Linux joystick interface initialization..."));

    /* open analog device files */
    const char *joydevs[][2] = {
        { "/dev/js0", "/dev/input/js0" },
        { "/dev/js1", "/dev/input/js1" },
        { "/dev/js2", "/dev/input/js2" },
        { "/dev/js3", "/dev/input/js3" },
        { "/dev/js4", "/dev/input/js4" },
        { "/dev/js5", "/dev/input/js5" }
    };

    for (i = 0; i < sizeof(joydevs) / sizeof(*joydevs); i++) {
        switch (joy_init_one (joydevs[i][0], i)) {
            case JOY_INIT_IOCTL_FAILED:
                return 0;
            case JOY_INIT_OK:
                break;
            case JOY_INIT_READ_ERROR:
                break;
            case JOY_INIT_FILE_NOT_FOUND:
                joy_init_one (joydevs[i][1], i);
        }
    }
}
