/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for PWM_0 */
#define PWM_0_INST                                                         TIMA1
#define PWM_0_INST_IRQHandler                                   TIMA1_IRQHandler
#define PWM_0_INST_INT_IRQN                                     (TIMA1_INT_IRQn)
#define PWM_0_INST_CLK_FREQ                                             32000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_0_C0_PORT                                                 GPIOB
#define GPIO_PWM_0_C0_PIN                                          DL_GPIO_PIN_2
#define GPIO_PWM_0_C0_IOMUX                                      (IOMUX_PINCM15)
#define GPIO_PWM_0_C0_IOMUX_FUNC                     IOMUX_PINCM15_PF_TIMA1_CCP0
#define GPIO_PWM_0_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_0_C1_PORT                                                 GPIOB
#define GPIO_PWM_0_C1_PIN                                          DL_GPIO_PIN_3
#define GPIO_PWM_0_C1_IOMUX                                      (IOMUX_PINCM16)
#define GPIO_PWM_0_C1_IOMUX_FUNC                     IOMUX_PINCM16_PF_TIMA1_CCP1
#define GPIO_PWM_0_C1_IDX                                    DL_TIMER_CC_1_INDEX

/* Defines for PWM_1 */
#define PWM_1_INST                                                         TIMA0
#define PWM_1_INST_IRQHandler                                   TIMA0_IRQHandler
#define PWM_1_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define PWM_1_INST_CLK_FREQ                                             32000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_1_C0_PORT                                                 GPIOA
#define GPIO_PWM_1_C0_PIN                                          DL_GPIO_PIN_8
#define GPIO_PWM_1_C0_IOMUX                                      (IOMUX_PINCM19)
#define GPIO_PWM_1_C0_IOMUX_FUNC                     IOMUX_PINCM19_PF_TIMA0_CCP0
#define GPIO_PWM_1_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_1_C1_PORT                                                 GPIOA
#define GPIO_PWM_1_C1_PIN                                          DL_GPIO_PIN_9
#define GPIO_PWM_1_C1_IOMUX                                      (IOMUX_PINCM20)
#define GPIO_PWM_1_C1_IOMUX_FUNC                     IOMUX_PINCM20_PF_TIMA0_CCP1
#define GPIO_PWM_1_C1_IDX                                    DL_TIMER_CC_1_INDEX



/* Defines for TIMER_1 */
#define TIMER_1_INST                                                     (TIMG7)
#define TIMER_1_INST_IRQHandler                                 TIMG7_IRQHandler
#define TIMER_1_INST_INT_IRQN                                   (TIMG7_INT_IRQn)
#define TIMER_1_INST_LOAD_VALUE                                             (9U)



/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                            4000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_4_MHZ_115200_BAUD                                        (2)
#define UART_0_FBRD_4_MHZ_115200_BAUD                                       (11)




/* Defines for SPI */
#define SPI_INST                                                           SPI1
#define SPI_INST_IRQHandler                                     SPI1_IRQHandler
#define SPI_INST_INT_IRQN                                         SPI1_INT_IRQn
#define GPIO_SPI_PICO_PORT                                                GPIOB
#define GPIO_SPI_PICO_PIN                                         DL_GPIO_PIN_8
#define GPIO_SPI_IOMUX_PICO                                     (IOMUX_PINCM25)
#define GPIO_SPI_IOMUX_PICO_FUNC                     IOMUX_PINCM25_PF_SPI1_PICO
/* GPIO configuration for SPI */
#define GPIO_SPI_SCLK_PORT                                                GPIOB
#define GPIO_SPI_SCLK_PIN                                         DL_GPIO_PIN_9
#define GPIO_SPI_IOMUX_SCLK                                     (IOMUX_PINCM26)
#define GPIO_SPI_IOMUX_SCLK_FUNC                     IOMUX_PINCM26_PF_SPI1_SCLK



/* Defines for ADC12_0 */
#define ADC12_0_INST                                                        ADC0
#define ADC12_0_INST_IRQHandler                                  ADC0_IRQHandler
#define ADC12_0_INST_INT_IRQN                                    (ADC0_INT_IRQn)
#define ADC12_0_ADCMEM_ADC_CH0                                DL_ADC12_MEM_IDX_0
#define ADC12_0_ADCMEM_ADC_CH0_REF               DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC12_0_ADCMEM_ADC_CH0_REF_VOLTAGE_V                                     3.3
#define GPIO_ADC12_0_C0_PORT                                               GPIOA
#define GPIO_ADC12_0_C0_PIN                                       DL_GPIO_PIN_27





/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOB)

/* Defines for B22: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED_B22_PIN                                             (DL_GPIO_PIN_22)
#define LED_B22_IOMUX                                            (IOMUX_PINCM50)
/* Port definition for Pin Group KEY */
#define KEY_PORT                                                         (GPIOB)

/* Defines for up: GPIOB.21 with pinCMx 49 on package pin 20 */
// groups represented: ["ENCODERB","KEY"]
// pins affected: ["E2A","E2B","up"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define KEY_up_IIDX                                         (DL_GPIO_IIDX_DIO21)
#define KEY_up_PIN                                              (DL_GPIO_PIN_21)
#define KEY_up_IOMUX                                             (IOMUX_PINCM49)
/* Defines for down: GPIOB.19 with pinCMx 45 on package pin 16 */
#define KEY_down_PIN                                            (DL_GPIO_PIN_19)
#define KEY_down_IOMUX                                           (IOMUX_PINCM45)
/* Defines for left: GPIOB.18 with pinCMx 44 on package pin 15 */
#define KEY_left_PIN                                            (DL_GPIO_PIN_18)
#define KEY_left_IOMUX                                           (IOMUX_PINCM44)
/* Defines for right: GPIOB.17 with pinCMx 43 on package pin 14 */
#define KEY_right_PIN                                           (DL_GPIO_PIN_17)
#define KEY_right_IOMUX                                          (IOMUX_PINCM43)
/* Port definition for Pin Group LCD */
#define LCD_PORT                                                         (GPIOB)

/* Defines for RES: GPIOB.10 with pinCMx 27 on package pin 62 */
#define LCD_RES_PIN                                             (DL_GPIO_PIN_10)
#define LCD_RES_IOMUX                                            (IOMUX_PINCM27)
/* Defines for DC: GPIOB.11 with pinCMx 28 on package pin 63 */
#define LCD_DC_PIN                                              (DL_GPIO_PIN_11)
#define LCD_DC_IOMUX                                             (IOMUX_PINCM28)
/* Defines for CS: GPIOB.14 with pinCMx 31 on package pin 2 */
#define LCD_CS_PIN                                              (DL_GPIO_PIN_14)
#define LCD_CS_IOMUX                                             (IOMUX_PINCM31)
/* Defines for BLK: GPIOB.26 with pinCMx 57 on package pin 28 */
#define LCD_BLK_PIN                                             (DL_GPIO_PIN_26)
#define LCD_BLK_IOMUX                                            (IOMUX_PINCM57)
/* Port definition for Pin Group Gray_Address */
#define Gray_Address_PORT                                                (GPIOA)

/* Defines for PIN_0: GPIOA.17 with pinCMx 39 on package pin 10 */
#define Gray_Address_PIN_0_PIN                                  (DL_GPIO_PIN_17)
#define Gray_Address_PIN_0_IOMUX                                 (IOMUX_PINCM39)
/* Defines for PIN_1: GPIOA.16 with pinCMx 38 on package pin 9 */
#define Gray_Address_PIN_1_PIN                                  (DL_GPIO_PIN_16)
#define Gray_Address_PIN_1_IOMUX                                 (IOMUX_PINCM38)
/* Defines for PIN_2: GPIOA.15 with pinCMx 37 on package pin 8 */
#define Gray_Address_PIN_2_PIN                                  (DL_GPIO_PIN_15)
#define Gray_Address_PIN_2_IOMUX                                 (IOMUX_PINCM37)
/* Port definition for Pin Group NRF24L01 */
#define NRF24L01_PORT                                                    (GPIOB)

/* Defines for CE: GPIOB.13 with pinCMx 30 on package pin 1 */
#define NRF24L01_CE_PIN                                         (DL_GPIO_PIN_13)
#define NRF24L01_CE_IOMUX                                        (IOMUX_PINCM30)
/* Defines for CSN: GPIOB.15 with pinCMx 32 on package pin 3 */
#define NRF24L01_CSN_PIN                                        (DL_GPIO_PIN_15)
#define NRF24L01_CSN_IOMUX                                       (IOMUX_PINCM32)
/* Port definition for Pin Group IIC_Software */
#define IIC_Software_PORT                                                (GPIOA)

/* Defines for SDA: GPIOA.0 with pinCMx 1 on package pin 33 */
#define IIC_Software_SDA_PIN                                     (DL_GPIO_PIN_0)
#define IIC_Software_SDA_IOMUX                                    (IOMUX_PINCM1)
/* Defines for SCL: GPIOA.1 with pinCMx 2 on package pin 34 */
#define IIC_Software_SCL_PIN                                     (DL_GPIO_PIN_1)
#define IIC_Software_SCL_IOMUX                                    (IOMUX_PINCM2)
/* Port definition for Pin Group ENCODERA */
#define ENCODERA_PORT                                                    (GPIOA)

/* Defines for E1A: GPIOA.25 with pinCMx 55 on package pin 26 */
// pins affected by this interrupt request:["E1A","E1B"]
#define ENCODERA_INT_IRQN                                       (GPIOA_INT_IRQn)
#define ENCODERA_INT_IIDX                       (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define ENCODERA_E1A_IIDX                                   (DL_GPIO_IIDX_DIO25)
#define ENCODERA_E1A_PIN                                        (DL_GPIO_PIN_25)
#define ENCODERA_E1A_IOMUX                                       (IOMUX_PINCM55)
/* Defines for E1B: GPIOA.26 with pinCMx 59 on package pin 30 */
#define ENCODERA_E1B_IIDX                                   (DL_GPIO_IIDX_DIO26)
#define ENCODERA_E1B_PIN                                        (DL_GPIO_PIN_26)
#define ENCODERA_E1B_IOMUX                                       (IOMUX_PINCM59)
/* Port definition for Pin Group ENCODERB */
#define ENCODERB_PORT                                                    (GPIOB)

/* Defines for E2A: GPIOB.20 with pinCMx 48 on package pin 19 */
#define ENCODERB_E2A_IIDX                                   (DL_GPIO_IIDX_DIO20)
#define ENCODERB_E2A_PIN                                        (DL_GPIO_PIN_20)
#define ENCODERB_E2A_IOMUX                                       (IOMUX_PINCM48)
/* Defines for E2B: GPIOB.24 with pinCMx 52 on package pin 23 */
#define ENCODERB_E2B_IIDX                                   (DL_GPIO_IIDX_DIO24)
#define ENCODERB_E2B_PIN                                        (DL_GPIO_PIN_24)
#define ENCODERB_E2B_IOMUX                                       (IOMUX_PINCM52)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_0_init(void);
void SYSCFG_DL_PWM_1_init(void);
void SYSCFG_DL_TIMER_1_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_SPI_init(void);
void SYSCFG_DL_ADC12_0_init(void);
void SYSCFG_DL_DMA_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
