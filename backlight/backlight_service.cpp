#include <cstdlib>
#include <cstdio>
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>
#include <chrono>
#include <thread>
#include <sys/syslog.h>
#include <pigpiod_if2.h>
 
using namespace std;
using namespace std::chrono;
 
#define PIN 34  // CPi-A/B/F/S
// #define PIN 44 // CPi-C
 
void log_info(const char *const message)
{
    printf("%s\n", message);
    syslog(LOG_INFO, "%s", message);
}
 
void log_err(const char *const message)
{
    printf("%s\n", message);
    syslog(LOG_ERR, "%s", message);
}
 
void backlight_on()
{
    auto instance = pigpio_start(NULL, NULL);
    if (instance != 0)
    {
        log_err("pigpio_start did not return success");
        return;
    }
 
    auto result = set_mode(instance, PIN, PI_OUTPUT);
    if (instance != 0)
    {
        log_err("set_mode did not return success");
        return;
    }
 
    result = gpio_write(instance, PIN, 1);
    if (instance != 0)
    {
        log_err("gpio_write did not return success");
        return;
    }
 
    pigpio_stop(instance);
}
 
void backlight_off()
{
    auto instance = pigpio_start(NULL, NULL);
    if (instance != 0)
    {
        log_err("pigpio_start did not return success");
        return;
    }
 
    auto result = set_mode(instance, PIN, PI_OUTPUT);
    if (instance != 0)
    {
        log_err("set_mode did not return success");
        return;
    }
 
    result = gpio_write(instance, PIN, 0);
    if (instance != 0)
    {
        log_err("gpio_write did not return success");
        return;
    }
 
    pigpio_stop(instance);
}
 
void set_backlight(CARD16 state)
{
    switch (state)
    {
        case DPMSModeOn:
            log_info("Turning backlight on");
            backlight_on();
            break;
        case DPMSModeOff:
            log_info("Turning backlight off");
            backlight_off();
            break;
        default:
            log_info("Invalid state");
            break;
    }
}
 
int main(int argc, char *argv[])
{
    openlog("backlight_service", LOG_NDELAY, LOG_USER);
 
    log_info("Starting");
 
    Display *dpy;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL)
    {
        log_err("Unable to open display");
        exit(EXIT_FAILURE);
    }
 
    BOOL onoff;
    CARD16 last_state;
    CARD16 state;
 
    // Initialize last_state
    if (!DPMSInfo(dpy, &state, &onoff))
    {
        log_err("DPMSInfo returned FALSE");
    }
    set_backlight(state);
    last_state = state;
 
    while(true)
    {
        if (!DPMSInfo(dpy, &state, &onoff))
        {
            log_err("DPMSInfo returned FALSE");
            this_thread::sleep_for(seconds(2));
            continue;
        }
 
        // If DPMS is not enabled, then display a message
        if (!onoff)
        {
            log_err("DPMSInfo is not enabled");
            this_thread::sleep_for(seconds(2));
            continue;
        }
 
        if (last_state != state)
        {
            set_backlight(state);
 
            last_state = state;
        }
        this_thread::sleep_for(milliseconds(200));
    }
 
    XCloseDisplay(dpy);
 
    log_info("Exiting");
 
    return 0;
}
