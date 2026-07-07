#include "ads1115.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ads1115";

#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG     0x01

static bool ads1115_initialized = false;

// Escribe un registro de 16 bits en el ADS1115
static esp_err_t ads1115_write_reg(uint8_t reg, uint16_t val)
{
    uint8_t data[3] = {
        reg,
        (uint8_t)((val >> 8) & 0xFF),
        (uint8_t)(val & 0xFF)
    };

    return i2c_master_write_to_device(
        I2C_BUS_PORT,
        ADS1115_ADDR_DEFAULT,
        data,
        sizeof(data),
        pdMS_TO_TICKS(100));
}

// Lee un registro de 16 bits del ADS1115
static esp_err_t ads1115_read_reg(uint8_t reg, uint16_t *val)
{
    uint8_t data[2] = {0};
    esp_err_t err = i2c_bus_read(ADS1115_ADDR_DEFAULT, reg, data, 2);
    if (err == ESP_OK) {
        *val = (uint16_t)((data[0] << 8) | data[1]);
    }
    return err;
}

bool ads1115_init(void)
{
    i2c_bus_init_if_needed();

    // Intentamos leer el registro de configuración para comprobar que el dispositivo responde
    uint16_t config = 0;
    esp_err_t err = ads1115_read_reg(ADS1115_REG_CONFIG, &config);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No se detecto el ADS1115 en la direccion I2C 0x%02X.", ADS1115_ADDR_DEFAULT);
        return false;
    }

    ads1115_initialized = true;
    ESP_LOGI(TAG, "ADS1115 detectado e inicializado correctamente (Config: 0x%04X).", config);
    return true;
}

bool ads1115_read_raw(uint8_t channel, int16_t *raw_val)
{
    if (!ads1115_initialized || channel > 3 || raw_val == NULL) {
        return false;
    }

    // Configuracion para single-shot en el canal indicado:
    // OS = 1 (iniciar conversion)
    // MUX: 100 para AIN0, 101 para AIN1, 110 para AIN2, 111 para AIN3
    // PGA = 001 (FSR = +-4.096V)
    // MODE = 1 (modo single-shot)
    // DR = 100 (128 SPS)
    // COMP_QUE = 11 (deshabilitar comparador)
    uint16_t config = 0x8000;              // Iniciar conversion
    config |= (uint16_t)((0x04 + channel) << 12);      // MUX
    config |= 0x0200;                      // PGA (+-4.096V)
    config |= 0x0100;                      // MODE (Single-Shot)
    config |= 0x0080;                      // DR (128 SPS)
    config |= 0x0003;                      // COMP_QUE (Deshabilitado)

    // Escribimos la configuracion para arrancar la conversion
    if (ads1115_write_reg(ADS1115_REG_CONFIG, config) != ESP_OK) {
        return false;
    }

    // Esperamos a que la conversion termine (en 128 SPS tarda ~8ms, esperamos 10ms)
    vTaskDelay(pdMS_TO_TICKS(10));

    uint16_t val = 0;
    if (ads1115_read_reg(ADS1115_REG_CONVERSION, &val) != ESP_OK) {
        return false;
    }

    *raw_val = (int16_t)val;
    return true;
}

float ads1115_raw_to_voltage(int16_t raw_val)
{
    // Rango FSR de +-4.096V mapeado en 16 bits con signo (-32768 a 32767)
    // LSB = 4.096V / 32768 = 0.125 mV / bit
    return (float)raw_val * 0.000125f;
}
