#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bmp280_probe.h"
#include "i2c_bus.h"

static const char *TAG = "bmp280_probe";

#define BMP280_ADDR_LOW 0x76
#define BMP280_ADDR_HIGH 0x77
#define BMP280_CALIB_START_REG 0x88
#define BMP280_CHIP_ID_REG 0xD0
#define BMP280_STATUS_REG 0xF3
#define BMP280_CTRL_MEAS_REG 0xF4
#define BMP280_CONFIG_REG 0xF5
#define BMP280_TEMP_MSB_REG 0xFA
#define BMP280_CHIP_ID 0x58
#define BME280_CHIP_ID 0x60

typedef struct {
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
} bmp280_calibration_t;

static bool bmp280_inicializado = false;
static uint8_t bmp280_addr = 0;
static int32_t bmp280_t_fine = 0;
static bmp280_calibration_t bmp280_calibration = {0};

static esp_err_t detectar_bmp280(uint8_t *detected_addr, uint8_t *chip_id)
{
    esp_err_t ret = i2c_bus_read_u8(BMP280_ADDR_LOW, BMP280_CHIP_ID_REG, chip_id);
    if (ret == ESP_OK) {
        *detected_addr = BMP280_ADDR_LOW;
        return ESP_OK;
    }

    ret = i2c_bus_read_u8(BMP280_ADDR_HIGH, BMP280_CHIP_ID_REG, chip_id);
    if (ret == ESP_OK) {
        *detected_addr = BMP280_ADDR_HIGH;
        return ESP_OK;
    }

    return ret;
}

static void cargar_calibracion(uint8_t *buffer)
{
    bmp280_calibration.dig_t1 = (uint16_t)((buffer[1] << 8) | buffer[0]);
    bmp280_calibration.dig_t2 = (int16_t)((buffer[3] << 8) | buffer[2]);
    bmp280_calibration.dig_t3 = (int16_t)((buffer[5] << 8) | buffer[4]);
    bmp280_calibration.dig_p1 = (uint16_t)((buffer[7] << 8) | buffer[6]);
    bmp280_calibration.dig_p2 = (int16_t)((buffer[9] << 8) | buffer[8]);
    bmp280_calibration.dig_p3 = (int16_t)((buffer[11] << 8) | buffer[10]);
    bmp280_calibration.dig_p4 = (int16_t)((buffer[13] << 8) | buffer[12]);
    bmp280_calibration.dig_p5 = (int16_t)((buffer[15] << 8) | buffer[14]);
    bmp280_calibration.dig_p6 = (int16_t)((buffer[17] << 8) | buffer[16]);
    bmp280_calibration.dig_p7 = (int16_t)((buffer[19] << 8) | buffer[18]);
    bmp280_calibration.dig_p8 = (int16_t)((buffer[21] << 8) | buffer[20]);
    bmp280_calibration.dig_p9 = (int16_t)((buffer[23] << 8) | buffer[22]);
}

static bool bmp280_configurar_medicion(void)
{
    uint8_t status = 0;

    if (i2c_bus_write_u8(bmp280_addr, BMP280_CONFIG_REG, 0x00) != ESP_OK) {
        return false;
    }

    if (i2c_bus_write_u8(bmp280_addr, BMP280_CTRL_MEAS_REG, 0x27) != ESP_OK) {
        return false;
    }

    for (int i = 0; i < 10; i++) {
        if (i2c_bus_read_u8(bmp280_addr, BMP280_STATUS_REG, &status) != ESP_OK) {
            return false;
        }

        if ((status & 0x09) == 0) {
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return true;
}

bool bmp280_init(void)
{
    uint8_t chip_id = 0;
    uint8_t calibration_raw[24] = {0};
    esp_err_t ret = ESP_FAIL;

    i2c_bus_init_if_needed();

    ret = detectar_bmp280(&bmp280_addr, &chip_id);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No pude leer el BMP280 por I2C.");
        ESP_LOGW(TAG, "Revisa VCC/GND, que CSB vaya a 3.3V, y que SDA/SCL realmente esten en GPIO6/GPIO7.");
        return false;
    }

    if (chip_id == BMP280_CHIP_ID) {
        ESP_LOGI(TAG, "BMP280 detectado correctamente en 0x%02X con chip ID 0x%02X.", bmp280_addr, chip_id);
    } else if (chip_id == BME280_CHIP_ID) {
        ESP_LOGW(TAG, "El sensor respondio en 0x%02X, pero su chip ID es 0x%02X: eso parece un BME280, no un BMP280.", bmp280_addr, chip_id);
    } else {
        ESP_LOGW(TAG, "Hay algo respondiendo en 0x%02X, pero el chip ID fue 0x%02X y no coincide con BMP280.", bmp280_addr, chip_id);
        return false;
    }

    ret = i2c_bus_read(bmp280_addr, BMP280_CALIB_START_REG, calibration_raw, sizeof(calibration_raw));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No pude leer la calibracion del BMP280.");
        return false;
    }

    cargar_calibracion(calibration_raw);

    if (!bmp280_configurar_medicion()) {
        ESP_LOGE(TAG, "No pude configurar la medicion del BMP280.");
        return false;
    }

    bmp280_inicializado = true;
    return true;
}

bool bmp280_read_temperature_c(float *temperature_c)
{
    return bmp280_read_temperature_and_pressure(temperature_c, NULL);
}

bool bmp280_read_temperature_and_pressure(float *temperature_c, float *pressure_hpa)
{
    uint8_t raw_data[6] = {0};
    int32_t adc_t;
    int32_t adc_p;
    int32_t var1;
    int32_t var2;
    int32_t temp_x100;
    int64_t p_var1;
    int64_t p_var2;
    int64_t pressure_q24_8;

    if (!bmp280_inicializado || temperature_c == NULL) {
        return false;
    }

    if (i2c_bus_read(bmp280_addr, 0xF7, raw_data, sizeof(raw_data)) != ESP_OK) {
        return false;
    }

    adc_p = (int32_t)((raw_data[0] << 12) | (raw_data[1] << 4) | (raw_data[2] >> 4));
    adc_t = (int32_t)((raw_data[3] << 12) | (raw_data[4] << 4) | (raw_data[5] >> 4));

    var1 = ((((adc_t >> 3) - ((int32_t)bmp280_calibration.dig_t1 << 1))) * ((int32_t)bmp280_calibration.dig_t2)) >> 11;
    var2 = (((((adc_t >> 4) - ((int32_t)bmp280_calibration.dig_t1)) *
              ((adc_t >> 4) - ((int32_t)bmp280_calibration.dig_t1))) >> 12) *
            ((int32_t)bmp280_calibration.dig_t3)) >> 14;

    bmp280_t_fine = var1 + var2;
    temp_x100 = (bmp280_t_fine * 5 + 128) >> 8;
    *temperature_c = temp_x100 / 100.0f;

    if (pressure_hpa == NULL) {
        return true;
    }

    p_var1 = ((int64_t)bmp280_t_fine) - 128000;
    p_var2 = p_var1 * p_var1 * (int64_t)bmp280_calibration.dig_p6;
    p_var2 = p_var2 + ((p_var1 * (int64_t)bmp280_calibration.dig_p5) << 17);
    p_var2 = p_var2 + (((int64_t)bmp280_calibration.dig_p4) << 35);
    p_var1 = ((p_var1 * p_var1 * (int64_t)bmp280_calibration.dig_p3) >> 8) +
             ((p_var1 * (int64_t)bmp280_calibration.dig_p2) << 12);
    p_var1 = (((((int64_t)1) << 47) + p_var1) * (int64_t)bmp280_calibration.dig_p1) >> 33;

    if (p_var1 == 0) {
        return false;
    }

    pressure_q24_8 = 1048576 - adc_p;
    pressure_q24_8 = (((pressure_q24_8 << 31) - p_var2) * 3125) / p_var1;
    p_var1 = (((int64_t)bmp280_calibration.dig_p9) * (pressure_q24_8 >> 13) * (pressure_q24_8 >> 13)) >> 25;
    p_var2 = (((int64_t)bmp280_calibration.dig_p8) * pressure_q24_8) >> 19;
    pressure_q24_8 = ((pressure_q24_8 + p_var1 + p_var2) >> 8) + (((int64_t)bmp280_calibration.dig_p7) << 4);

    *pressure_hpa = (float)pressure_q24_8 / 25600.0f;
    return true;
}
