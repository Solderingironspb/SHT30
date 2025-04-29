#include "SHT30.h"

// clang-format off
static uint8_t SHT30_COMMAND[2] = {0x24, 00};  // Repeatability Hight, Clock stretching disabled
uint8_t SHT30_DATA[6] = {0,};   //Данные, которые будем читать с датчика
uint8_t CRC8_Data[2] = {0,};    //Данные для проверки CRC
// clang-format on


void SHT30_Task_Timer_update(struct SHT30_struct *p) {
    if (p->Task_timer) {
        p->Task_timer--;
    }
}

bool SHT30_Get_Data(I2C_TypeDef* I2C, struct SHT30_struct *p)  {
    uint16_t Data = 0;
    uint8_t CRC_Check = 0;
    if ((p->Task == SHT30_Task_Transmit) && (p->Task_timer == 0)) {
        CMSIS_I2C_Data_Transmit(I2C, SHT30_ADDR, SHT30_COMMAND, 2, 100);
        p->Task = SHT30_Task_Receive;
        p->Task_timer = 20;
    } else if ((p->Task == SHT30_Task_Receive) && (p->Task_timer == 0)) {
        CMSIS_I2C_Data_Receive(I2C, SHT30_ADDR, SHT30_DATA, 6, 100);
        // Получение данных по температуре
        CRC8_Data[0] = SHT30_DATA[0];
        CRC8_Data[1] = SHT30_DATA[1];
        CRC_Check = CRC8_Bytes_Calculate_software(CRC8_Data, 2, SHT30_CRC_INITIALIZATION, SHT30_CRC_POLYNOMIAL, SHT30_CRC_REFLECT_INPUT, SHT30_CRC_REFLECT_OUTPUT, SHT30_CRC_FINAL_XOR);
        if (SHT30_DATA[2] == CRC_Check) {
            Data = ((uint16_t)SHT30_DATA[0] << 8U) | SHT30_DATA[1];
            p->Temperature = -45 + (175 * (float)Data / 0xFFFF);
        } else {
            return false;
        }
        // Получение данных по влажности
        CRC8_Data[0] = SHT30_DATA[3];
        CRC8_Data[1] = SHT30_DATA[4];
        CRC_Check = CRC8_Bytes_Calculate_software(CRC8_Data, 2, SHT30_CRC_INITIALIZATION, SHT30_CRC_POLYNOMIAL, SHT30_CRC_REFLECT_INPUT, SHT30_CRC_REFLECT_OUTPUT, SHT30_CRC_FINAL_XOR);
        if (SHT30_DATA[5] == CRC_Check) {
            Data = ((uint16_t)SHT30_DATA[3] << 8U) | SHT30_DATA[4];
            p->Humidity = (float)Data / 0xFFFF * 100.0f;
        } else {
            return false;
        }
        p->Task = SHT30_Task_Transmit;
        p->Task_timer = 0;
    }
    return true;
}
