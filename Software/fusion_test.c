#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// ---------- Konstanten ----------
#define I2C_DEV "/dev/i2c-1"
#define ADS1115_ADDRESS 0x48
#define PCF8574_ADDRESS 0x20

// ---------- ADS1115 Register ----------
#define ADS1115_REG_POINTER_CONVERT 0x00
#define ADS1115_REG_POINTER_CONFIG  0x01

#define ADS1115_CONFIG_OS_SINGLE     0x8000
#define ADS1115_CONFIG_GAIN_2_3      0x0000
#define ADS1115_CONFIG_MODE_SINGLE   0x0100
#define ADS1115_CONFIG_DR_128SPS     0x0080
#define ADS1115_CONFIG_COMP_QUE_DIS  0x0003
#define ADS1115_CONFIG_MUX_SINGLE_0  0x4000
#define ADS1115_CONFIG_MUX_SINGLE_1  0x5000

// ---------- Globale Variablen ----------
int fd_ads = -1;
int fd_pcf = -1;
uint8_t _writebuf = 0xFF;

// ---------- Prototypen ----------
void initialisierung();
float spannungmessen();
float strommessen();
void ausgangsetzen(int pin0, int pin1);
int einganglesen();

// ---------- Hilfsfunktionen ----------
void writeRegister(int fd, uint8_t reg, uint16_t value) {
    uint8_t buf[3] = {reg, value >> 8, value & 0xFF};
    write(fd, buf, 3);
}

uint16_t readRegister(int fd, uint8_t reg) {
    uint8_t buf[2];
    write(fd, &reg, 1);
    read(fd, buf, 2);
    return (buf[0] << 8) | buf[1];
}

void waitConversionComplete(int fd) {
    while (!(readRegister(fd, ADS1115_REG_POINTER_CONFIG) & 0x8000)) {
        usleep(1000);
    }
}

float rawToVolts(int16_t raw) {
    return raw * 6.144f / 32768.0f;
}

// ---------- Implementierung ----------

void initialisierung() {
    // ADS1115 initialisieren
    fd_ads = open(I2C_DEV, O_RDWR);
    if (fd_ads < 0 || ioctl(fd_ads, I2C_SLAVE, ADS1115_ADDRESS) < 0) {
        perror("ADS1115 Init");
        _exit(1);
    }

    // PCF8574 initialisieren
    fd_pcf = open(I2C_DEV, O_RDWR);
    if (fd_pcf < 0 || ioctl(fd_pcf, I2C_SLAVE, PCF8574_ADDRESS) < 0) {
        perror("PCF8574 Init");
        _exit(1);
    }

    // Setze Ausgang 0 & 1 auf Output (ziehen auf 0)
    _writebuf &= ~(1 << 0); // laden
    _writebuf &= ~(1 << 1); // entladen
    _writebuf |=  (1 << 2); // start als Eingang (1 setzen)
    write(fd_pcf, &_writebuf, 1);
}

float spannungmessen() {
    uint16_t config = ADS1115_CONFIG_OS_SINGLE |
                      ADS1115_CONFIG_MUX_SINGLE_0 |
                      ADS1115_CONFIG_GAIN_2_3 |
                      ADS1115_CONFIG_MODE_SINGLE |
                      ADS1115_CONFIG_DR_128SPS |
                      ADS1115_CONFIG_COMP_QUE_DIS;
    writeRegister(fd_ads, ADS1115_REG_POINTER_CONFIG, config);
    waitConversionComplete(fd_ads);
    int16_t raw = (int16_t)readRegister(fd_ads, ADS1115_REG_POINTER_CONVERT);
    return rawToVolts(raw);
}

float strommessen() {
    uint16_t config = ADS1115_CONFIG_OS_SINGLE |
                      ADS1115_CONFIG_MUX_SINGLE_1 |
                      ADS1115_CONFIG_GAIN_2_3 |
                      ADS1115_CONFIG_MODE_SINGLE |
                      ADS1115_CONFIG_DR_128SPS |
                      ADS1115_CONFIG_COMP_QUE_DIS;
    writeRegister(fd_ads, ADS1115_REG_POINTER_CONFIG, config);
    waitConversionComplete(fd_ads);
    int16_t raw = (int16_t)readRegister(fd_ads, ADS1115_REG_POINTER_CONVERT);
    return rawToVolts(raw);
}

void ausgangsetzen(int pin0, int pin1) {
    if (pin0) _writebuf |= (1 << 0);
    else _writebuf &= ~(1 << 0);

    if (pin1) _writebuf |= (1 << 1);
    else _writebuf &= ~(1 << 1);

    write(fd_pcf, &_writebuf, 1);
}

int einganglesen() {
    uint8_t readbuf;
    read(fd_pcf, &readbuf, 1);
    return (readbuf & (1 << 2)) ? 1 : 0;
}

// ---------- Hauptprogramm ----------
int main() {
    initialisierung();

    while (1) {
        float spannung = spannungmessen();
        float strom = strommessen();
        int start = einganglesen();

        ausgangsetzen(start, !start);

        printf("Spannung: %.3f V | Strom: %.3f V | Start: %d\n", spannung, strom, start);
        sleep(1);
    }

    return 0;
}
