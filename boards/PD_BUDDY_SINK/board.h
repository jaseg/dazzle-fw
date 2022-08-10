/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#ifndef BOARD_H
#define BOARD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*
 * Setup for PD Buddy Sink board.
 */

/*
 * Board identifier.
 */
#define BOARD_DAZZLE_RGB
#define BOARD_NAME                  "Dazzle RGB Multimedia Keys"

/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0U
#endif

#define STM32_LSEDRV                (3U << 3U)

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                8000000U
#endif

#define STM32_HSE_BYPASS

/*
 * MCU type as defined in the ST header.
 */
#define STM32F072xB

/*
 * IO pins assignments.
 */
#define GPIOA_ENC_SW                0U
#define GPIOA_SR_STROBE             1U
#define GPIOA_SR_CLR                2U
#define GPIOA_BIGBUCK_EN            3U
#define GPIOA_PIN4                  4U
#define GPIOA_PIN5                  5U
#define GPIOA_ENC_A                 6U
#define GPIOA_ENC_B                 7U
#define GPIOA_TLC_XLAT              8U
#define GPIOA_TLC_MODE              9U
#define GPIOA_TLC_BLANK             10U
#define GPIOA_USB_DM                11U
#define GPIOA_USB_DP                12U
#define GPIOA_SWDIO                 13U
#define GPIOA_SWCLK                 14U
#define GPIOA_PIN15                 15U

#define GPIOB_PUSB_VBUS_SENSE       0U
#define GPIOB_DUSB_VBUS_SENSE       1U
#define GPIOB_MATRIX_ROW1           2U
#define GPIOB_TLC_SCK               3U
#define GPIOB_MATRIX_ROW0           4U
#define GPIOB_TLC_DIN               5U
#define GPIOB_MATRIX_COL0           6U
#define GPIOB_MATRIX_COL1           7U
#define GPIOB_MATRIX_COL2           8U
#define GPIOB_MATRIX_COL3           9U
#define GPIOB_SCL                   10U
#define GPIOB_SDA                   11U
#define GPIOB_INT_N                 12U
#define GPIOB_SR_SCK                13U
#define GPIOB_PIN14                 14U
#define GPIOB_SR_DIN                15U

#define GPIOC_PIN13                 13U
#define GPIOC_PIN14                 14U
#define GPIOC_PIN15                 15U

/*
 * IO lines assignments.
 */
#define LINE_ENC_SW                 PAL_LINE(GPIOA, GPIOA_ENC_SW)
#define LINE_SR_STROBE              PAL_LINE(GPIOA, GPIOA_SR_STROBE)
#define LINE_SR_CLR                 PAL_LINE(GPIOA, GPIOA_SR_CLR)
#define LINE_SR_SCK                 PAL_LINE(GPIOB, GPIOB_SR_SCK)
#define LINE_SR_DIN                 PAL_LINE(GPIOB, GPIOB_SR_DIN)
#define LINE_BIGBUCK_EN             PAL_LINE(GPIOA, GPIOA_BIGBUCK_EN)
#define LINE_ENC_A                  PAL_LINE(GPIOA, GPIOA_ENC_A)
#define LINE_ENC_B                  PAL_LINE(GPIOA, GPIOA_ENC_B)
#define LINE_TLC_XLAT               PAL_LINE(GPIOA, GPIOA_TLC_XLAT)
#define LINE_TLC_MODE               PAL_LINE(GPIOA, GPIOA_TLC_MODE)
#define LINE_TLC_BLANK              PAL_LINE(GPIOA, GPIOA_TLC_BLANK)
#define LINE_TLC_SCK                PAL_LINE(GPIOB, GPIOA_TLC_SCK)
#define LINE_TLC_DIN                PAL_LINE(GPIOB, GPIOA_TLC_DIN)
#define LINE_USB_DM                 PAL_LINE(GPIOA, GPIOA_USB_DM)
#define LINE_USB_DP                 PAL_LINE(GPIOA, GPIOA_USB_DP)
#define LINE_SWDIO                  PAL_LINE(GPIOA, GPIOA_SWDIO)
#define LINE_SWCLK                  PAL_LINE(GPIOA, GPIOA_SWCLK)
#define LINE_PUSB_VBUS_SENSE        PAL_LINE(GPIOB, GPIOB_PUSB_VBUS_SENSE)
#define LINE_DUSB_VBUS_SENSE        PAL_LINE(GPIOB, GPIOB_DUSB_VBUS_SENSE)
#define LINE_MATRIX_ROW0            PAL_LINE(GPIOB, GPIOB_MATRIX_ROW0)
#define LINE_MATRIX_ROW1            PAL_LINE(GPIOB, GPIOB_MATRIX_ROW1)
#define LINE_MATRIX_COL0            PAL_LINE(GPIOB, GPIOB_MATRIX_COL0)
#define LINE_MATRIX_COL1            PAL_LINE(GPIOB, GPIOB_MATRIX_COL1)
#define LINE_MATRIX_COL2            PAL_LINE(GPIOB, GPIOB_MATRIX_COL2)
#define LINE_MATRIX_COL3            PAL_LINE(GPIOB, GPIOB_MATRIX_COL3)
#define LINE_SCL                    PAL_LINE(GPIOB, GPIOB_SCL)
#define LINE_SDA                    PAL_LINE(GPIOB, GPIOB_SDA)
#define LINE_INT_N                  PAL_LINE(GPIOB, GPIOB_INT_N)


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2U))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2U))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2U))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2U))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_VERYLOW(n)       (0U << ((n) * 2U))
#define PIN_OSPEED_LOW(n)           (1U << ((n) * 2U))
#define PIN_OSPEED_MEDIUM(n)        (2U << ((n) * 2U))
#define PIN_OSPEED_HIGH(n)          (3U << ((n) * 2U))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2U))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2U))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2U))
#define PIN_AFIO_AF(n, v)           ((v) << (((n) % 8U) * 4U))

/* GPIOA */

#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_ENC_SW) |\
                                     PIN_MODE_OUTPUT(GPIOA_SR_STROBE) |\
                                     PIN_MODE_ALTERNATE(GPIOA_SR_CLR) |\
                                     PIN_MODE_OUTPUT(GPIOA_BIGBUCK_EN) |\
                                     PIN_MODE_INPUT(GPIOA_PIN4) |\
                                     PIN_MODE_INPUT(GPIOA_PIN5) |\
                                     PIN_MODE_ALTERNATE(GPIOA_ENC_A) |\
                                     PIN_MODE_ALTERNATE(GPIOA_ENC_B) |\
                                     PIN_MODE_ALTERNATE(GPIOA_TLC_XLAT) |\
                                     PIN_MODE_ALTERNATE(GPIOA_TLC_MODE) |\
                                     PIN_MODE_ALTERNATE(GPIOA_TLC_BLANK) |\
                                     PIN_MODE_INPUT(GPIOA_USB_DM) |\
                                     PIN_MODE_INPUT(GPIOA_USB_DP) |\
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |\
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |\
                                     PIN_MODE_OUTPUT(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (0)
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_HIGH(GPIOA_SR_STROBE) |\
                                     PIN_OSPEED_HIGH(GPIOA_SR_CLR) |\
                                     PIN_OSPEED_HIGH(GPIOA_BIGBUCK_EN) |\
                                     PIN_OSPEED_HIGH(GPIOA_TLC_XLAT) |\
                                     PIN_OSPEED_HIGH(GPIOA_TLC_MODE) |\
                                     PIN_OSPEED_HIGH(GPIOA_TLC_BLANK) |\
                                     PIN_OSPEED_HIGH(GPIOA_SWDIO) |\
                                     PIN_OSPEED_HIGH(GPIOA_SWCLK))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_PULLUP(GPIOA_ENC_SW) |\
                                     PIN_PUPDR_PULLUP(GPIOA_ENC_A) |\
                                     PIN_PUPDR_PULLUP(GPIOA_ENC_B) |\
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |\
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK))
#define VAL_GPIOA_ODR               (0)
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_SR_STROBE, 5U) |\
                                     PIN_AFIO_AF(GPIOA_SR_CLR, 0U) |\
                                     PIN_AFIO_AF(GPIOA_ENC_A, 1U) |\
                                     PIN_AFIO_AF(GPIOA_ENC_B, 1U))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_TLC_XLAT, 2U) |\
                                     PIN_AFIO_AF(GPIOA_TLC_MODE, 2U) |\
                                     PIN_AFIO_AF(GPIOA_TLC_BLANK, 2U) |\
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0U) |\
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0U))

/* GPIOB */

#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_PUSB_VBUS_SENSE) |\
                                     PIN_MODE_ANALOG(GPIOB_DUSB_VBUS_SENSE) |\
                                     PIN_MODE_OUTPUT(GPIOB_MATRIX_ROW1) |\
                                     PIN_MODE_ALTERNATE(GPIOB_TLC_SCK) |\
                                     PIN_MODE_OUTPUT(GPIOB_MATRIX_ROW0) |\
                                     PIN_MODE_ALTERNATE(GPIOB_TLC_DIN) |\
                                     PIN_MODE_INPUT(GPIOB_MATRIX_COL0) |\
                                     PIN_MODE_INPUT(GPIOB_MATRIX_COL1) |\
                                     PIN_MODE_INPUT(GPIOB_MATRIX_COL2) |\
                                     PIN_MODE_INPUT(GPIOB_MATRIX_COL3) |\
                                     PIN_MODE_ALTERNATE(GPIOB_SCL) |\
                                     PIN_MODE_ALTERNATE(GPIOB_SDA) |\
                                     PIN_MODE_INPUT(GPIOB_INT_N) |\
                                     PIN_MODE_ALTERNATE(GPIOB_SR_SCK) |\
                                     PIN_MODE_INPUT(GPIOB_PIN14) |\
                                     PIN_MODE_ALTERNATE(GPIOB_SR_DIN))
#define VAL_GPIOB_OTYPER            (0)
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_HIGH(GPIOB_MATRIX_ROW1) |\
                                     PIN_OSPEED_HIGH(GPIOB_TLC_SCK) |\
                                     PIN_OSPEED_HIGH(GPIOB_MATRIX_ROW0) |\
                                     PIN_OSPEED_HIGH(GPIOB_TLC_DIN) |\
                                     PIN_OSPEED_HIGH(GPIOB_SCL) |\
                                     PIN_OSPEED_HIGH(GPIOB_SDA) |\
                                     PIN_OSPEED_HIGH(GPIOB_SR_SCK) |\
                                     PIN_OSPEED_HIGH(GPIOB_SR_DIN))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_PULLUP(GPIOB_MATRIX_COL0) |\
                                     PIN_PUPDR_PULLUP(GPIOB_MATRIX_COL1) |\
                                     PIN_PUPDR_PULLUP(GPIOB_MATRIX_COL2) |\
                                     PIN_PUPDR_PULLUP(GPIOB_MATRIX_COL3) |\
                                     PIN_PUPDR_PULLUP(GPIOB_INT_N))
#define VAL_GPIOB_ODR               (0)
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_TLC_SCK, 0U) |\
                                     PIN_AFIO_AF(GPIOB_TLC_DIN, 0U))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_SCL, 1U) |\
                                     PIN_AFIO_AF(GPIOB_SDA, 1U) |\
                                     PIN_AFIO_AF(GPIOB_SR_SCK, 0U) |\
                                     PIN_AFIO_AF(GPIOB_SR_DIN, 0U))

/* GPIOC */

#define VAL_GPIOC_MODER             (PIN_MODE_INPUT(GPIOC_PIN13) |\
                                     PIN_MODE_INPUT(GPIOC_PIN14) |\
                                     PIN_MODE_INPUT(GPIOC_PIN15))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_PIN13) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN14) |\
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN15))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_VERYLOW(GPIOC_PIN13) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN14) |\
                                     PIN_OSPEED_VERYLOW(GPIOC_PIN15))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_PULLDOWN(GPIOC_PIN13) |\
                                     PIN_PUPDR_PULLUP(GPIOC_PIN14) |\
                                     PIN_PUPDR_PULLUP(GPIOC_PIN15))
#define VAL_GPIOC_ODR               (PIN_ODR_HIGH(GPIOC_PIN13) |\
                                     PIN_ODR_HIGH(GPIOC_PIN14) |\
                                     PIN_ODR_HIGH(GPIOC_PIN15))
#define VAL_GPIOC_AFRL              (0)
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_PIN13, 0U) |\
                                     PIN_AFIO_AF(GPIOC_PIN14, 0U) |\
                                     PIN_AFIO_AF(GPIOC_PIN15, 0U))

/* ChibiOS boilerplate */

#define VAL_GPIOD_MODER             (0)
#define VAL_GPIOD_OTYPER            (0)
#define VAL_GPIOD_OSPEEDR           (0)
#define VAL_GPIOD_PUPDR             (0)
#define VAL_GPIOD_ODR               (0)
#define VAL_GPIOD_AFRL              (0)
#define VAL_GPIOD_AFRH              (0)

#define VAL_GPIOE_MODER             (0)
#define VAL_GPIOE_OTYPER            (0)
#define VAL_GPIOE_OSPEEDR           (0)
#define VAL_GPIOE_PUPDR             (0)
#define VAL_GPIOE_ODR               (0)
#define VAL_GPIOE_AFRL              (0)
#define VAL_GPIOE_AFRH              (0)

#define VAL_GPIOF_MODER             (0)
#define VAL_GPIOF_OTYPER            (0)
#define VAL_GPIOF_OSPEEDR           (0)
#define VAL_GPIOF_PUPDR             (0)
#define VAL_GPIOF_ODR               (0)
#define VAL_GPIOF_AFRL              (0)
#define VAL_GPIOF_AFRH              (0)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* BOARD_H */
