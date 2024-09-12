#include <string>
#include <cstdio>
#include <unistd.h> // sleep
#include <sys/stat.h> //stat

const std::string PWM_SYSFS = "/sys/class/pwm/pwmchip0/";
const std::string PWM0_PATH = "/sys/class/pwm/pwmchip0/pwm0/";
const std::string PWM1_PATH = "/sys/class/pwm/pwmchip0/pwm1/";

static void writeSYS(const std::string& filename, const std::uint32_t value){
    FILE* fp;           // create a file pointer fp
    fp = fopen(filename.c_str(), "w");  // open file for writing
    fprintf(fp, "%u", value);   // send the value to the file
    fclose(fp);         // close the file using fp
}

int main(int argc, char* argv[]){
    std::uint32_t dimming_pct = 0;
    struct stat buf;
    const std::string exportfile = PWM_SYSFS+"export";
    const std::string enablefile = PWM1_PATH+"enable";
    const std::string periodfile = PWM1_PATH+"period";
    const std::string dutycyclefile = PWM1_PATH+"duty_cycle";

    if (argc != 2) {
	return -1;
    }
    dimming_pct = strtoul(argv[1],NULL,0);
    if (stat(enablefile.c_str(),&buf) < 0) {
      writeSYS(exportfile, 1);  // export PWM channel (1)
      sleep(1);           // sleep for 100ms
    }
    if ((stat(enablefile.c_str(),&buf) < 0) || (buf.st_size == 0)) {
	return -1;
    }
    writeSYS(enablefile , 0);     // disable
    usleep(1000);
    writeSYS(periodfile , 1000000);     // set period : 1kHz
    usleep(1000);
    writeSYS(dutycyclefile , dimming_pct*10000); // set duty_cycle : 50% = dimming
    usleep(1000);
    writeSYS(enablefile , 1);     // enable
    
    return 0;
}
