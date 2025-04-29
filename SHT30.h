/**
 ******************************************************************************
 *  @file SHT30.h
 *  @brief Библиотека для работы датчика температуры и влажности SHT30. CRC-8 присутствует
 *  @author Волков Олег
 *  @date 29.04.2025
 *
 *  YouTube: https://www.youtube.com/channel/UCzZKTNVpcMSALU57G1THoVw
 *  GitHub: https://github.com/Solderingironspb
 *  Группа ВК: https://vk.com/solderingiron.stm32
 *  Про CRC на википедии: https://ru.wikipedia.org/wiki/%D0%A6%D0%B8%D0%BA%D0%BB%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%B8%D0%B9_%D0%B8%D0%B7%D0%B1%D1%8B%D1%82%D0%BE%D1%87%D0%BD%D1%8B%D0%B9_%D0%BA%D0%BE%D0%B4
 *  Урок по CRC: https://youtu.be/YyQonUJrBn4
 *  Для проверки себя: http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
 *
 ******************************************************************************
 */

/**
 ******************************************************************************
 * Как работать с данной библиотекой:
 * Адрес SHT30 на шине I2C может быть либо 0x44, если ножка ADDR подтянута к земле,
 * либо 0x45, если ножка ADDR подтянута к питанию.
 * Функция считывания данных здесь одна и предусматривает самый 
 * точный режим Repeatability Hight (высокая повторяемость), а также режим без задерживания
 * тактового сигнала I2C во время измерений. Время измерения при режиме Repeatability Hight 
 * составляет примерно 15 мс. Я на всякий случай использую 20 мс.
 *
 * Создаем структуру датчика, например struct SHT30_struct SHT30_1;
 * Далее, в том же бесконечном цикле вставляем функцию SHT30_Get_Data(I2C1, &SHT30_1);
 * где I2C1 - это I2C, который мы подключили к данному датчику.
 *     &SHT30_1 - структура, которую мы создавали под датчик.
 * Данная функция сама понимает, в какой момент времени ей отправлять данные датчику,
 * а в какой момент забирать данные с датчика.
 * Этим функционалом у нас управляет таймер с выбором задачи. Он у нас находится в структуре SHT30_1.
 * Таймер должен обновляться каждую 1 мс. Поэтому можно взять вторую функцию SHT30_Task_Timer_update(&SHT30_1);
 * и вставить ее в прерывание от системного таймера SysTick.
 * 
 * По итогу имеем код в main.c:
 *  int main(void) {
 *  CMSIS_Debug_init();
 *  CMSIS_RCC_SystemClock_72MHz();
 *  CMSIS_SysTick_Timer_init();
 *  CMSIS_I2C1_Init();  // PB7 SDA, PB6 SCK
 *
 *    while (1) {
 *        SHT30_Get_Data(I2C1, &SHT30_1);
 *    }
 *  }
 *
 * А вот так выглядить прерывание системного таймера:
 * #include "SHT30.h"
 * extern struct SHT30_struct SHT30_1;
 *
 * void SysTick_Handler(void) { 
 *	  SysTimer_ms++;
 *	  if (Delay_counter_ms) {
 *		  Delay_counter_ms--;
 *	  }
 *	  if (Timeout_counter_ms) {
 *	    	Timeout_counter_ms--;
 * 	  }
 *    SHT30_Task_Timer_update(&SHT30_1);
 * }
 *
 */
#ifndef INC_SHT30_H_
#define INC_SHT30_H_

#include "CRC_Software_calculation.h"
#include "stm32f103xx_CMSIS.h"

#define SHT30_ADDR 0x44  // Адрес устройства на шине I2C

/*Вводные данные для расчета CRC-8*/
#define SHT30_CRC_POLYNOMIAL     0x31
#define SHT30_CRC_INITIALIZATION 0xFF
#define SHT30_CRC_REFLECT_INPUT  false
#define SHT30_CRC_REFLECT_OUTPUT false
#define SHT30_CRC_FINAL_XOR      0x00

enum {
    SHT30_Task_Transmit,  //  Задача на передачу данных датчику
    SHT30_Task_Receive    //  Задача на примем данных от датчика
};

struct SHT30_struct {
    float Temperature;    // Температура
    float Humidity;       // Влажность
    uint16_t Task_timer;  // Таймер для разделения задач на передачу данных и прием данных
    uint8_t Task;         // Выбранная задача (прием/передача)
};

void SHT30_Task_Timer_update(struct SHT30_struct *p);
bool SHT30_Get_Data(I2C_TypeDef *I2C, struct SHT30_struct *p);

#endif /* INC_EASY_TIMER_H_ */