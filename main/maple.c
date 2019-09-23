#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_intr_alloc.h"
#include "driver/gpio.h"
#include "esp32/dport_access.h"
#include "maple.h"

#define DEBUG  (1ULL << 25)
#define MAPLE0 (1ULL << 26)
#define MAPLE1 (1ULL << 27)
#define TIMEOUT 8

#define MAPLE_FUNC_DATA_CTRL 0x3FFFFF
#define wait_100ns() asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
#define maple_fix_byte(s, a, b) (s ? ((a << s) | (b >> (8 - s))) : b)

static struct io *output;
const char dev_name_ctrl[] = "BlueRetro Adapter - Controller";
const char dev_name_mem[] = "BlueRetro Adapter - Memory";
const char dev_name_rumble[] = "BlueRetro Adapter - Rumble";
const char dev_license[] = "Jacques Gagnon IoT";

uint8_t dev_info[] =
{
    0x1C, 0x20, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x72, 0x44, 0x00, 0xFF, 0x63, 0x6D, 0x61, 0x65, 0x20, 0x74, 0x73, 0x61,
    0x74, 0x6E, 0x6F, 0x43, 0x6C, 0x6C, 0x6F, 0x72, 0x20, 0x20, 0x72, 0x65, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x64, 0x6F, 0x72, 0x50, 0x64, 0x65, 0x63, 0x75, 0x20, 0x79, 0x42, 0x20,
    0x55, 0x20, 0x72, 0x6F, 0x72, 0x65, 0x64, 0x6E, 0x63, 0x69, 0x4C, 0x20, 0x65, 0x73, 0x6E, 0x65,
    0x6F, 0x72, 0x46, 0x20, 0x45, 0x53, 0x20, 0x6D, 0x45, 0x20, 0x41, 0x47, 0x52, 0x45, 0x54, 0x4E,
    0x53, 0x49, 0x52, 0x50, 0x4C, 0x2C, 0x53, 0x45, 0x20, 0x2E, 0x44, 0x54, 0x20, 0x20, 0x20, 0x20,
    0x01, 0xF4, 0x01, 0xAE, 0x00
};

uint8_t status[] =
{
    0x03, 0x20, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x00
};

uint32_t intr_cnt = 0;

uint8_t buffer[544] = {0};

static void IRAM_ATTR maple_tx(uint8_t *data, uint8_t len) {
    uint8_t *crc = data + (len - 1);

    ets_delay_us(55);

    GPIO.out_w1ts = MAPLE0 | MAPLE1;
    gpio_set_direction(26, GPIO_MODE_OUTPUT);
    gpio_set_direction(27, GPIO_MODE_OUTPUT);
    DPORT_STALL_OTHER_CPU_START();
    GPIO.out_w1tc = MAPLE0;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;
    wait_100ns();
    wait_100ns();

    for (uint32_t bit = 0; bit < len*8; ++data) {
        for (uint32_t mask = 0x80; mask; mask >>= 1, ++bit) {
            GPIO.out_w1ts = MAPLE0;
            wait_100ns();
            wait_100ns();
            if (*data & mask) {
                GPIO.out_w1ts = MAPLE1;
            }
            else {
                GPIO.out_w1tc = MAPLE1;
            }
            wait_100ns();
            GPIO.out_w1tc = MAPLE0;
            wait_100ns();
            wait_100ns();
            mask >>= 1;
            ++bit;
            GPIO.out_w1ts = MAPLE1;
            wait_100ns();
            wait_100ns();
            if (*data & mask) {
                GPIO.out_w1ts = MAPLE0;
            }
            else {
                GPIO.out_w1tc = MAPLE0;
            }
            wait_100ns();
            GPIO.out_w1tc = MAPLE1;
            wait_100ns();
            wait_100ns();
        }
        *crc ^= *data;
    }
    GPIO.out_w1ts = MAPLE0;
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE1;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE0;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE0;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1tc = MAPLE0;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE0;
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    wait_100ns();
    GPIO.out_w1ts = MAPLE1;

    gpio_set_direction(26, GPIO_MODE_INPUT);
    gpio_set_direction(27, GPIO_MODE_INPUT);
    DPORT_STALL_OTHER_CPU_END();
    /* Send start sequence */

}

static void IRAM_ATTR maple_rx(void* arg)
{
    const uint32_t gpio_intr_status = GPIO.acpu_int;
    uint32_t timeout = 0;
    uint32_t bit_cnt = 0;
    uint32_t gpio;
    uint8_t *data = buffer;
    uint32_t byte;
    uint8_t cmd;

    if (gpio_intr_status) {
        DPORT_STALL_OTHER_CPU_START();
        GPIO.out_w1tc = DEBUG;
        while (1) {
            do {
                while (!(GPIO.in & MAPLE0));
                while (((gpio = GPIO.in) & MAPLE0));
                if (gpio & MAPLE1) {
                    *data = (*data << 1) + 1;
                }
                else {
                    *data <<= 1;
                }
                ++bit_cnt;
                //GPIO.out_w1ts = DEBUG;
                while (!(GPIO.in & MAPLE1));
                timeout = 0;
                while (((gpio = GPIO.in) & MAPLE1)) {
                    if (++timeout > TIMEOUT) {
                        goto maple_end;
                    }
                }
                if (gpio & MAPLE0) {
                    *data = (*data << 1) + 1;
                }
                else {
                    *data <<= 1;
                }
                ++bit_cnt;
                //GPIO.out_w1tc = DEBUG;
            } while ((bit_cnt % 8));
            ++data;
        }
maple_end:
        DPORT_STALL_OTHER_CPU_END();
        GPIO.out_w1ts = DEBUG;
        byte = ((bit_cnt - 1) / 8);

        cmd = maple_fix_byte((bit_cnt - 1) % 8, buffer[2], buffer[3]);
        switch (cmd) {
            case 0x01:
                maple_tx(dev_info, sizeof(dev_info));
                if (!output->format) {
                    output->format = IO_FORMAT_DC;
                    memcpy((uint8_t *)&output->io.dc, status + 8, sizeof(output->io.dc));
                }
                break;
            case 0x09:
                memcpy(status + 8, (uint8_t *)&output->io.dc, sizeof(output->io.dc));
                maple_tx(status, sizeof(status));
                ++output->poll_cnt;
                break;
            default:
                ets_printf("Unsupported cmd: 0x%02X\n", cmd);
                break;

        }

        GPIO.status_w1tc = gpio_intr_status;
    }
}

void init_maple(struct io *output_data)
{
    output = output_data;

    gpio_config_t io_conf0 = {
        .intr_type = GPIO_PIN_INTR_NEGEDGE,
        .pin_bit_mask = MAPLE0,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_config_t io_conf1 = {
        .intr_type = 0,
        .pin_bit_mask = MAPLE1,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_config_t io_conf2 = {
        .intr_type = 0,
        .pin_bit_mask = DEBUG,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };

    gpio_config(&io_conf0);
    gpio_config(&io_conf1);
    gpio_config(&io_conf2);
    GPIO.out_w1ts = DEBUG;

    esp_intr_alloc(ETS_GPIO_INTR_SOURCE, ESP_INTR_FLAG_LEVEL3, maple_rx, NULL, NULL);
}
