#include <stdio.h>
#include <stdint.h>
#include <unistd.h>    // usleep
#include <fcntl.h>     // open
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// ADS1115 Register-Adressen
#define ADS1115_REG_POINTER_CONVERT   0x00
#define ADS1115_REG_POINTER_CONFIG    0x01

// ADS1115 I2C Adresse (Standard)
#define ADS1115_ADDRESS 0x48

// Konfigurationsbits (vereinfacht)
#define ADS1115_CONFIG_OS_SINGLE      0x8000  // Start single conversion
#define ADS1115_CONFIG_MUX_SINGLE_0   0x4000  // Kanal 0
#define ADS1115_CONFIG_MUX_SINGLE_1   0x5000  // Kanal 1
#define ADS1115_CONFIG_GAIN_2_3       0x0000  // Gain = +/-6.144V
#define ADS1115_CONFIG_MODE_SINGLE    0x0100  // Einzelmessung
#define ADS1115_CONFIG_DR_128SPS      0x0080  // Datenrate 128 SPS
#define ADS1115_CONFIG_COMP_QUE_DIS   0x0003  // Comparator deaktivieren

// Funktion um 16-Bit Wert im Big-Endian zu schreiben
void writeRegister(int fd, uint8_t reg, uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = (value >> 8) & 0xFF;   // MSB
    buffer[2] = value & 0xFF;          // LSB
    write(fd, buffer, 3);
}

// Funktion um 16-Bit Wert im Big-Endian zu lesen
uint16_t readRegister(int fd, uint8_t reg) {
    uint8_t buffer[2];
    write(fd, &reg, 1);
    read(fd, buffer, 2);
    return (buffer[0] << 8) | buffer[1];
}

// Wartefunktion, bis Konvertierung fertig ist (Polling)
void waitConversionComplete(int fd) {
    while (1) {
        uint16_t config = readRegister(fd, ADS1115_REG_POINTER_CONFIG);
        if (config & 0x8000) // OS Bit = 1 wenn fertig
            break;
        usleep(1000); // 1 ms warten
    }
}

// Wandelt Rohwert in Volt um bei Gain +/-6.144 V und 16 bit Auflösung
float rawToVolts(int16_t raw) {
    return (float)raw * 6.144f / 32768.0f;
}

int main() {
    const char *i2c_device = "/dev/i2c-1";

    // I2C Gerät öffnen
    int fd = open(i2c_device, O_RDWR);
    if (fd < 0) {
        perror("Öffnen von i2c Device fehlgeschlagen");
        return 1;
    }

    // I2C Slave Adresse setzen
    if (ioctl(fd, I2C_SLAVE, ADS1115_ADDRESS) < 0) {
        perror("Setzen der I2C Adresse fehlgeschlagen");
        close(fd);
        return 1;
    }

    while(1) {
        // Kanal 0 starten
        uint16_t config0 = ADS1115_CONFIG_OS_SINGLE |
                           ADS1115_CONFIG_MUX_SINGLE_0 |
                           ADS1115_CONFIG_GAIN_2_3 |
                           ADS1115_CONFIG_MODE_SINGLE |
                           ADS1115_CONFIG_DR_128SPS |
                           ADS1115_CONFIG_COMP_QUE_DIS;
        writeRegister(fd, ADS1115_REG_POINTER_CONFIG, config0);
        waitConversionComplete(fd);
        int16_t raw0 = (int16_t)readRegister(fd, ADS1115_REG_POINTER_CONVERT);
        float volts0 = rawToVolts(raw0);

        // Kanal 1 starten
        uint16_t config1 = ADS1115_CONFIG_OS_SINGLE |
                           ADS1115_CONFIG_MUX_SINGLE_1 |
                           ADS1115_CONFIG_GAIN_2_3 |
                           ADS1115_CONFIG_MODE_SINGLE |
                           ADS1115_CONFIG_DR_128SPS |
                           ADS1115_CONFIG_COMP_QUE_DIS;
        writeRegister(fd, ADS1115_REG_POINTER_CONFIG, config1);
        waitConversionComplete(fd);
        int16_t raw1 = (int16_t)readRegister(fd, ADS1115_REG_POINTER_CONVERT);
        float volts1 = rawToVolts(raw1);
        float amps = 9.88*volts1 - 25.27;
        amps = amps*1000;

        // Ausgabe
        printf("Kanal 0: %6d -> %.4f V    |    Kanal 1: %6d -> %.4f mA\n", raw0, volts0, raw1, amps);

        sleep(1); // 1 Sekunde warten
    }

    close(fd);
    return 0;
}
