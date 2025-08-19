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

//Zustände der Zustandsmaschine
int Z_START				 = 1;
int Z_SPANNUNG_MESSEN_1	 = 2;
int Z_LADEN_AN			 = 3;
int Z_LADEN_AUS			 = 4;
int Z_ZAEHLER_ERHOEHEN	 = 5;
int Z_ENTLADEN_AN		 = 6;
int Z_ENTLADEN_AUS		 = 7;
int Z_SPANNUNG_MESSEN_2	 = 8;
int Z_STROM_MESSEN		 = 9;
int Z_AUFSUMMIEREN		 = 10;
int Z_ENDE				 = 11;

//Initialzustand
int fertig = 0;
int zustand = 1;
int wiederholung = 0;
int entladezeitzaehler = 0;
int starttaster = 0;
int laden = 0;
int entladen = 0;
float spannung = 0.0;
float strom = 0.0;
float ladung = 0.0;
int messwert = 0;

//Konstanten
const float SOLL_LADESPANNUNG = 14.4;
const float SOLL_ENTLADESPANNUNG = 12.0;
const int MAX_WIEDERHOLUNGEN = 5;
const int MAX_ENTLADEZEIT = 60;
#define MAX_MESSWERTE 20

float messwertspeicher[MAX_MESSWERTE] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

//Zustandsfunktionen
void initialisierung();
float spannungmessen();
float strommessen();
void ausgangsetzen(int pin0, int pin1);
int einganglesen();
void zaehlererhoehen(int *zaehler);
void stromsummieren(float *ladung, float strom);
void writeRegister(int fd, uint8_t reg, uint16_t value);
uint16_t readRegister(int fd, uint8_t reg);
void waitConversionComplete(int fd);
float rawToVolts(int16_t raw);

//Zustandsübergang
int zustandsuebergang(int alterzustand){
	int neuerzustand = Z_START;
	if(alterzustand == Z_START && starttaster == 1){
		neuerzustand = Z_SPANNUNG_MESSEN_1;
	}
	if(alterzustand == Z_SPANNUNG_MESSEN_1 && spannung < SOLL_LADESPANNUNG){
		neuerzustand = Z_LADEN_AN;
	}
	if(alterzustand == Z_LADEN_AN){
		neuerzustand = Z_LADEN_AUS;
	}
	if(alterzustand == Z_LADEN_AUS){
		neuerzustand = Z_SPANNUNG_MESSEN_1;
	}
	if(alterzustand == Z_SPANNUNG_MESSEN_1 && spannung >= SOLL_LADESPANNUNG){
		neuerzustand = Z_ZAEHLER_ERHOEHEN;
	}
	if(alterzustand == Z_ZAEHLER_ERHOEHEN && wiederholung <= MAX_WIEDERHOLUNGEN){
		neuerzustand = Z_ENTLADEN_AN;
	}
	if(alterzustand == Z_ENTLADEN_AN && entladezeitzaehler == MAX_ENTLADEZEIT){
		neuerzustand = Z_ENTLADEN_AUS;
	}
	if(alterzustand == Z_ENTLADEN_AUS){
		neuerzustand = Z_SPANNUNG_MESSEN_2;
	}
	if(alterzustand == Z_SPANNUNG_MESSEN_2 && spannung > SOLL_ENTLADESPANNUNG){
		neuerzustand = Z_ENTLADEN_AN;
	}
	if(alterzustand == Z_SPANNUNG_MESSEN_2 && spannung <= SOLL_ENTLADESPANNUNG){
		neuerzustand = Z_LADEN_AN;
	}
	if(alterzustand == Z_ENTLADEN_AN && entladezeitzaehler < MAX_ENTLADEZEIT){
		neuerzustand = Z_STROM_MESSEN;
	}	
	if(alterzustand == Z_STROM_MESSEN){
		neuerzustand = Z_AUFSUMMIEREN;
	}		
	if(alterzustand == Z_AUFSUMMIEREN){
		neuerzustand = Z_ENTLADEN_AN;
	}		
	if(alterzustand == Z_ZAEHLER_ERHOEHEN && wiederholung == MAX_WIEDERHOLUNGEN){
		neuerzustand = Z_ENDE;
	}		
	
	return neuerzustand;
}

//Ausgangslogik
void ausgangslogik(int aktuellerzustand){
	if(aktuellerzustand == Z_START){
		printf("Zustand: Z_START\n");
		starttaster = einganglesen();
		printf("taster: %d\n\n", starttaster);
	}
	if(aktuellerzustand == Z_SPANNUNG_MESSEN_1){
		printf("Zustand: Z_SPANNUNG_MESSEN_1\n");
		spannung = spannungmessen();
		printf("Gemessene Spannung: %.3f V\n\n", spannung);
	}
	if(aktuellerzustand == Z_LADEN_AN){
		printf("Zustand: Z_LADEN_AN\n\n");
		ausgangsetzen(1,0);
		sleep(60);
	}
	if(aktuellerzustand == Z_LADEN_AUS){
		printf("Zustand: Z_LADEN_AUS\n\n");
		ausgangsetzen(0,0);
		//Schaltzeit
		usleep(20000);
	}
	if(aktuellerzustand == Z_ZAEHLER_ERHOEHEN){
		printf("Zustand: Z_ZAEHLER_ERHOEHEN\n");
		zaehlererhoehen(&wiederholung);
		printf("Wiederholung: %d\n\n", wiederholung);
	}
	if(aktuellerzustand == Z_ENTLADEN_AN){
		printf("Zustand: Z_ENTLADEN_AN\n\n");
		ausgangsetzen(0,1);
	}
	if(aktuellerzustand == Z_ENTLADEN_AUS){
		printf("Zustand: Z_ENTLADEN_AUS\n\n");
		ausgangsetzen(0,0);
                entladezeitzaehler = 0;
                messwertspeicher[messwert] = ladung;
                messwert += 1;
                ladung = 0.0;
		//Schaltzeit
		usleep(20000);
	}
	if(aktuellerzustand == Z_SPANNUNG_MESSEN_2){
		printf("Zustand: Z_SPANNUNG_MESSEN_2\n");
		spannung = spannungmessen();
		printf("Gemessene Spannung: %.3f V\n\n", spannung);
	}
	if(aktuellerzustand == Z_STROM_MESSEN){
		printf("Zustand: Z_STROM_MESSEN\n");
		strom = strommessen();
		printf("Gemessener Strom: %.3f A\n\n", strom);
	}
	if(aktuellerzustand == Z_AUFSUMMIEREN){
		printf("Zustand: Z_AUFSUMMIEREN\n");
		stromsummieren(&ladung, strom);
		printf("Ladung: %.3f As\n\n", ladung);
                entladezeitzaehler += 1;
                usleep(1000000);
	}
	if(aktuellerzustand == Z_ENDE){
		printf("Zustand: Z_ENDE\n\n");
                printf("Messwerte aus Speicher:\n");
                for(int i = 0; i < messwert; i++){
                     printf("Messwert %d: %.3f As\n", i, messwertspeicher[i]);
                }
		fertig = 1;
	}
}

int main() {
	initialisierung();
	do{
		ausgangslogik(zustand);
		zustand = zustandsuebergang(zustand);
	}while(!fertig);
	return 0;
}

void initialisierung(){
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
float spannungmessen(){
	uint16_t config = ADS1115_CONFIG_OS_SINGLE |
                      ADS1115_CONFIG_MUX_SINGLE_0 |
                      ADS1115_CONFIG_GAIN_2_3 |
                      ADS1115_CONFIG_MODE_SINGLE |
                      ADS1115_CONFIG_DR_128SPS |
                      ADS1115_CONFIG_COMP_QUE_DIS;
    writeRegister(fd_ads, ADS1115_REG_POINTER_CONFIG, config);
    waitConversionComplete(fd_ads);
    int16_t raw = (int16_t)readRegister(fd_ads, ADS1115_REG_POINTER_CONVERT);
    return 4.35*rawToVolts(raw); //Skallierungsfaktor moeglicherweise anpassen -> abhaengig vom Spannungsteiler
}
float strommessen(){
	uint16_t config = ADS1115_CONFIG_OS_SINGLE |
                      ADS1115_CONFIG_MUX_SINGLE_1 |
                      ADS1115_CONFIG_GAIN_2_3 |
                      ADS1115_CONFIG_MODE_SINGLE |
                      ADS1115_CONFIG_DR_128SPS |
                      ADS1115_CONFIG_COMP_QUE_DIS;
    writeRegister(fd_ads, ADS1115_REG_POINTER_CONFIG, config);
    waitConversionComplete(fd_ads);
    int16_t raw = (int16_t)readRegister(fd_ads, ADS1115_REG_POINTER_CONVERT);
    float volts = rawToVolts(raw);
    float amps = 9.88*volts - 25.27;
    return amps*1000;
}
void ausgangsetzen(int pin0, int pin1){
	if (pin0) _writebuf |= (1 << 0);
    else _writebuf &= ~(1 << 0);

    if (pin1) _writebuf |= (1 << 1);
    else _writebuf &= ~(1 << 1);

    write(fd_pcf, &_writebuf, 1);
}
int einganglesen(){
	uint8_t readbuf;
    read(fd_pcf, &readbuf, 1);
    return (readbuf & (1 << 2)) ? 1 : 0;
}
void zaehlererhoehen(int *zaehler){
	(*zaehler)++;
}
void stromsummieren(float *ladung, float strom){
	(*ladung) += strom;
}
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
