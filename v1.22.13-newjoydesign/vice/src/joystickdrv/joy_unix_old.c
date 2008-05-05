#include <sys/types.h>
#include <fcntl.h>

#include "config.h"
#include "log.h"
#include "lib.h"

#if defined LINUX_JOYSTICK
#include <linux/joystick.h>

#elif defined BSD_JOYSTICK

#ifdef HAVE_MACHINE_JOYSTICK_H
#include <machine/joystick.h>
#endif
#ifdef HAVE_SYS_JOYSTICK_H
#include <sys/joystick.h>
#endif
#define JS_DATA_TYPE joystick
#define JS_RETURN    sizeof(struct joystick)

#else
#error Unknown Joystick

#endif /* LINUX_JOYSTICK, BSD_JOYSTICK */

#define JOYCALLOOPS 100
#define JOYSENSITIVITY 5

#include "joystick.h"

static log_t joystick_log = LOG_ERR;

typedef struct joy_unix_old_priv_s {
    int fd;
    int joyxmin;
    int joyxmax;
    int joyymin;
    int joyymax;
} joy_unix_old_priv_t;

static void old_joystick(int i, void* dev)
{
    int status;
    struct JS_DATA_TYPE js;
    joy_unix_old_priv_t* priv = (joy_unix_old_priv_t*)dev;

    status = read(priv->fd, &js, JS_RETURN);
    if (status != JS_RETURN) {
        log_error(joystick_log,
	    _("Error reading joystick device."));
    return;
    } 
    joystick_set_value_absolute(i, 0);

    if (js.y < priv->joyymin)
	joystick_set_value_or(i, 1);
    if (js.y > priv->joyymax)
	joystick_set_value_or(i, 2);
    if (js.x < priv->joyxmin)
	joystick_set_value_or(i, 4);
    if (js.x > priv->joyxmax)
        joystick_set_value_or(i, 8);
#ifdef LINUX_JOYSTICK
    if (js.buttons)
        joystick_set_value_or(i, 16);
#elif defined(BSD_JOYSTICK)
    if (js.b1 || js.b2)
        joystick_set_value_or(i, 16);
#endif
}

static void old_joystick_destroy(void* dev)
{
    joy_unix_old_priv_t* priv = (joy_unix_old_priv_t*)dev;
    close(priv->fd);
}

static joy_device_t old_unix_joystick_device = {
    old_joystick,
    NULL,
    NULL,
    old_joystick_destroy
} ;


static void joy_init_one (const char* devname, int port) {
    int j;
    int ajoyfd = open(devname, O_RDONLY);
    joy_unix_old_priv_t* priv;
    joy_data_t* dev;
    int joyxcal = 0;
    int joyycal = 0;

    if (ajoyfd < 0) {
        log_warning(joystick_log,
                    _("Cannot open joystick device `%s'."), devname);
        return;
    } 
        
    priv = (joy_unix_old_priv_t*)lib_calloc(1, sizeof(joy_unix_old_priv_t));

    /* calibration loop */
    for (j = 0; j < JOYCALLOOPS; j++) {
        struct JS_DATA_TYPE js;
        int status = read(ajoyfd, &js, JS_RETURN);

        if (status != JS_RETURN) {
            log_warning(joystick_log,
                        _("Error reading joystick device `%s'."), devname);
            lib_free(priv);
            return;
        } 
                
        /* determine average */
        joyxcal += js.x;
        joyycal += js.y;
    }

    /* correct average */
    joyxcal /= JOYCALLOOPS;
    joyycal /= JOYCALLOOPS;

    /* determine tresholds */
    priv->joyxmin = joyxcal - joyxcal / JOYSENSITIVITY;
    priv->joyxmax = joyxcal + joyxcal / JOYSENSITIVITY;
    priv->joyymin = joyycal - joyycal / JOYSENSITIVITY;
    priv->joyymax = joyycal + joyycal / JOYSENSITIVITY;

    log_message(joystick_log,
        _("Hardware joystick calibration for device `%s':"), devname);
    log_message(joystick_log,
        "  X: min: %i , mid: %i , max: %i.",
        priv->joyxmin, joyxcal, priv->joyxmax);
    log_message(joystick_log,
        "  Y: min: %i , mid: %i , max: %i.",
        priv->joyymin, joyycal, priv->joyymax);
    
    dev = (joy_data_t*)lib_malloc(sizeof(joy_device_t));
    priv->fd = ajoyfd;
    dev->priv = priv;
    dev->name = lib_msprintf("Hardware joystick (old style API) in port %u", port);
    dev->device = &old_unix_joystick_device;
    
    joystick_register_device(dev);
}

void old_joystick_init(void)
{
    int i;

    joystick_log = log_open("Joystick");

    /* open analog device files */
    joy_init_one(
#ifdef LINUX_JOYSTICK
        "/dev/js0",
#elif defined(BSD_JOYSTICK)
        "/dev/joy0",
#endif
                 0);

    joy_init_one(
#ifdef LINUX_JOYSTICK
        "/dev/js1",
#elif defined(BSD_JOYSTICK)
        "/dev/joy1",
#endif
                 1);
}
