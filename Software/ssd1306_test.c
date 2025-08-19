#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_ADDR 0x3C
#define I2C_DEV "/dev/i2c-1"

#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)
uint8_t buffer[SSD1306_BUFFER_SIZE];

int ssd1306_fd = -1;

// Low-Level
void ssd1306_send_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd};
    if (write(ssd1306_fd, buf, 2) != 2) perror("write command");
}

void ssd1306_send_data(uint8_t *data, size_t len) {
    uint8_t *buf = malloc(len + 1);
    buf[0] = 0x40;
    memcpy(buf + 1, data, len);
    if (write(ssd1306_fd, buf, len + 1) != (ssize_t)(len + 1)) perror("write data");
    free(buf);
}

// Init wie bei Adafruit
void ssd1306_init() {
    ssd1306_send_command(0xAE);
    ssd1306_send_command(0xD5); ssd1306_send_command(0x80);
    ssd1306_send_command(0xA8); ssd1306_send_command(0x3F);
    ssd1306_send_command(0xD3); ssd1306_send_command(0x00);
    ssd1306_send_command(0x40);
    ssd1306_send_command(0x8D); ssd1306_send_command(0x14);
    ssd1306_send_command(0x20); ssd1306_send_command(0x00);
    ssd1306_send_command(0xA1);
    ssd1306_send_command(0xC8);
    ssd1306_send_command(0xDA); ssd1306_send_command(0x12);
    ssd1306_send_command(0x81); ssd1306_send_command(0xCF);
    ssd1306_send_command(0xD9); ssd1306_send_command(0xF1);
    ssd1306_send_command(0xDB); ssd1306_send_command(0x40);
    ssd1306_send_command(0xA4);
    ssd1306_send_command(0xA6);
    ssd1306_send_command(0x2E); // disable scroll
    ssd1306_send_command(0xAF);
}

void ssd1306_clear_display() {
    memset(buffer, 0, sizeof(buffer));
}

void ssd1306_draw_pixel(int x, int y, int color) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;
    int index = x + (y / 8) * SSD1306_WIDTH;
    uint8_t mask = 1 << (y % 8);
    if (color)
        buffer[index] |= mask;
    else
        buffer[index] &= ~mask;
}

void ssd1306_display() {
    // Set explicit column and page range to prevent wraparound
    ssd1306_send_command(0x21); // Set column address
    ssd1306_send_command(0x00); // Start column: 0
    ssd1306_send_command(0x7F); // End column: 127

    ssd1306_send_command(0x22); // Set page address
    ssd1306_send_command(0x00); // Start page: 0
    ssd1306_send_command(0x07); // End page: 7

    for (uint8_t page = 0; page < SSD1306_HEIGHT / 8; page++) {
        ssd1306_send_command(0xB0 + page);
        ssd1306_send_command(0x00);
        ssd1306_send_command(0x10);
        ssd1306_send_data(&buffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
    }
}

int main() {
    ssd1306_fd = open(I2C_DEV, O_RDWR);
    if (ssd1306_fd < 0) {
        perror("Failed to open I2C");
        return 1;
    }
    if (ioctl(ssd1306_fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
        perror("Failed to select I2C device");
        close(ssd1306_fd);
        return 1;
    }

    ssd1306_init();
    ssd1306_clear_display();

    // Beispiel: Diagonale Linie
    for (int i = 0; i < 64; i++) {
        ssd1306_draw_pixel(i, i, 1);
    }

    ssd1306_display();
    close(ssd1306_fd);
    return 0;
}

