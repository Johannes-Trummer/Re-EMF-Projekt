#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>

#define PCF8574_ADDR 0x20
#define I2C_DEV "/dev/i2c-1"

// PCF8574 verwendet open-drain:
// Bit = 1 -> Pin als Input mit Pullup (logisch HIGH)
// Bit = 0 -> Pin als Output und LOW

static int fd = -1;
static uint8_t _writebuf = 0xFF;  // alle Pins Input/Pullup (HIGH)

// Init I2C und Adresse setzen
bool pcf8574_begin() {
    fd = open(I2C_DEV, O_RDWR);
    if (fd < 0) {
        perror("Open I2C device");
        return false;
    }
    if (ioctl(fd, I2C_SLAVE, PCF8574_ADDR) < 0) {
        perror("Set I2C address");
        close(fd);
        return false;
    }
    // Schreibe initialen Zustand (alle Input)
    if (write(fd, &_writebuf, 1) != 1) {
        perror("Initial write");
        close(fd);
        return false;
    }
    return true;
}

// PinMode: INPUT=1 (Pullup), OUTPUT=0 (Low)
bool pcf8574_pinMode(uint8_t pinnum, uint8_t mode) {
    if (pinnum > 7) return false;

    if (mode == 1) { // INPUT oder INPUT_PULLUP
        _writebuf |= (1 << pinnum);
    } else { // OUTPUT
        _writebuf &= ~(1 << pinnum);
    }
    if (write(fd, &_writebuf, 1) != 1) {
        perror("pinMode write");
        return false;
    }
    return true;
}

// digitalWrite: true -> HIGH (Input/Pullup), false -> LOW (Output)
bool pcf8574_digitalWrite(uint8_t pinnum, bool val) {
    if (pinnum > 7) return false;

    if (val) {
        _writebuf |= (1 << pinnum);
    } else {
        _writebuf &= ~(1 << pinnum);
    }
    if (write(fd, &_writebuf, 1) != 1) {
        perror("digitalWrite write");
        return false;
    }
    return true;
}

// digitalRead: liest aktuelle Pins vom Expander
bool pcf8574_digitalRead(uint8_t pinnum, bool *val) {
    if (pinnum > 7) return false;

    uint8_t readbuf = 0;
    if (read(fd, &readbuf, 1) != 1) {
        perror("digitalRead read");
        return false;
    }
    *val = (readbuf & (1 << pinnum)) ? true : false;
    return true;
}

void pcf8574_close() {
    if (fd >= 0) close(fd);
}

int main() {
    if (!pcf8574_begin()) return 1;

    // Pin0 und Pin1 als OUTPUT, HIGH setzen (also Input/Pullup)
    pcf8574_pinMode(0, 0); // OUTPUT (LOW)
    pcf8574_digitalWrite(0, false); // Setze Pin0 auf HIGH -> Input/Pullup (wegen open-drain) //macht Probleme

    pcf8574_pinMode(1, 0); // OUTPUT (LOW)
    pcf8574_digitalWrite(1, false); // Setze Pin1 auf HIGH -> Input/Pullup (wegen open-drain) //Entladen

    pcf8574_close();
    return 0;
}
