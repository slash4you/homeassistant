#include <string>
#include <unistd.h> // sleep
#include <sys/stat.h> //stat
#include <cstdio>
#include <cstdint>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h> // open


const std::string PWM_SYSFS = "/sys/class/pwm/pwmchip0/";
const std::string PWM0_PATH = "/sys/class/pwm/pwmchip0/pwm0/";
const std::string PWM1_PATH = "/sys/class/pwm/pwmchip0/pwm1/";
const std::string exportfile = PWM_SYSFS+"export";
const std::string enablefile = PWM1_PATH+"enable";
const std::string periodfile = PWM1_PATH+"period";
const std::string dutycyclefile = PWM1_PATH+"duty_cycle";

std::int32_t writeSYS(const std::string& filename, const std::uint32_t value){
    std::int32_t retval = 0;
    FILE* fp = fopen(filename.c_str(), "w");  // open file for writing
    if (fp == nullptr) {
      retval = -1;
    } else {
      fprintf(fp, "%u", value);   // send the value to the file
      fclose(fp);         // close the file using fp
    }
    return retval;
}

std::int32_t dim_screen(std::uint32_t percent)
{
    std::int32_t retval = 0;
    struct stat buf;

    if (stat(enablefile.c_str(),&buf) < 0) {
      retval |= writeSYS(exportfile, 1);  // export PWM channel (1)
      sleep(1);                           // sleep for 1s needed at startup
    }
    if ((stat(enablefile.c_str(),&buf) < 0) || (buf.st_size == 0)) {
      retval = -1;
    } else {
      retval |= writeSYS(enablefile , 0);     // pwm disable
      //usleep(1000);
      retval |= writeSYS(periodfile , 1000000);     // set period : 1kHz
      //usleep(1000);
      retval |= writeSYS(dutycyclefile , percent*10000); // set duty_cycle = set dimming percent
      //usleep(1000);
      retval |= writeSYS(enablefile , 1);     // pwm enable
    } // if stat

    return retval;
}

std::int32_t main(int argc, char* argv[])
{
    std::uint32_t backlight_timeout_ms = 60000;
    const char * backlight_timeout_ms_str = nullptr;
    std::int32_t input_fd = -1;
    std::int32_t retval = 0;

    // dimming level set to 0 by default
    std::int32_t rc = dim_screen(0);
    if (rc < 0) {
      retval = -1;
      goto exit1;
    }

    backlight_timeout_ms_str = ::getenv( "BACKLIGHT_TIMEOUT_MS" );
    if ( backlight_timeout_ms_str != nullptr) {
      backlight_timeout_ms = ::strtoul(backlight_timeout_ms_str,NULL,0);
    }

    // Touch events happen on event0
    input_fd = ::open("/dev/input/event0", O_RDONLY);

    if (input_fd <= 0) {
      retval = -2;
      goto exit1;
    }

    while (true)
    {
    	fd_set read_fdset;
	struct timeval timeout;
    	struct input_event ev;

        timeout.tv_sec = backlight_timeout_ms / 1000;
        timeout.tv_usec = (backlight_timeout_ms % 1000) * 1000;

        FD_ZERO(&read_fdset);
        FD_SET(input_fd, &read_fdset);

eintr:
        rc = ::select(input_fd + 1, &read_fdset, NULL, NULL, &timeout);
	if (rc < 0 && errno == EINTR) {
	  goto eintr;
	} else if (rc < 0) {
	  // error on select
	  retval = -3;
	  goto exit2;
	} else if (rc == 0) {
	  // timeout
          if (dim_screen(100) < 0) {
            retval = -4;
            goto exit2;
	  }
        } else {
	  // success
	  if (FD_ISSET(input_fd, &read_fdset)) {

	    // get the input event
            auto size = read(input_fd, &ev, sizeof(ev));

            // If an input event
            if (size == sizeof(ev)) {
              if (ev.type == EV_KEY && ev.value == 1) {
	         // if a touch
                 if (dim_screen(0) < 0) {
		   retval = -5;
                   goto exit2;
	         }
              }
            } else {
	      // bad event size
	      retval = -6;
	      goto exit2;
	    }

	  } else {
	    // missed event
	    retval = -7;
	    goto exit2;
	  }
        } // if select

    } // while

exit2:
    ::close(input_fd);
exit1:
    return retval;
}
