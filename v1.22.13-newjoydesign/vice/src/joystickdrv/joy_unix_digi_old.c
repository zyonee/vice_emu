typedef struct joy_old_digi_priv_s {
    int fd;
} joy_old_digi_priv_t;

static void old_digital_joystick(int i, joy_device_t* dev) {
            int status;
            struct DJS_DATA_TYPE djs;
    joy_old_digi_priv_t* priv = (joy_unix_priv_t*)dev->priv;
    BYTE newval;

                status = read(priv->fd, &djs, DJS_RETURN);
                if (status != DJS_RETURN) {
                    log_error(joystick_log,
                              _("Error reading digital joystick device."));
                    return;
                } 

                    newval =  ((~(djs.switches >> 3)) & 0x1f);
                    joystick_set_value_absolute(i, newval);
}

static void old_digital_joystick_close(joy_device_t* dev)
{
    joy_unix_priv_t* priv = (joy_unix_priv_t*)dev->priv
    close(priv->fd);
}

void old_digital_joystick_init(void) {
    for (i = 0; i < 2; i++) {
        const char *dev = (i == 0) ? "/dev/djs0" : "/dev/djs1";
        int djoyfd = open(dev, O_RDONLY);
        if (djoyfd < 0)
            log_message(joystick_log,
                        _("Cannot open joystick device `%s'."), dev);
        else {
            *dev = lib_malloc(sizeof(joy_device_t));
            priv = lib_malloc(sizeof(joy_old_digi_priv_t));
            priv->fd = djoyfd;
            dev->poll = old_digital_joystick;
            dev->close = old_digital_joystick_close;
            dev->name = lib_msprintf("Digital joystick (Linux 0.8.0.2) in port %u", i);
            dev->priv = priv;
            joystick_register_dev(dev);
        }
    }
}
