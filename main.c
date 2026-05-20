/*
 * @file main.c
 * @brief Parking sensor
 *
 * Ultra-sonic HC-SR04 sensor is used to send impulse
 * of ~10us and then capture echo with timer CAPTURE
 * mode from objects in form of a impulse, distance
 * is in proportion with length of echo impulse.
 * If distance is smaller, diodes are switching faster
 * If distance is smaller than DISTANCE_TRESHOLD, diode
 * is on.
 * Low power regime of msp430 is used. Microcontroller
 * goes out of low power regime when receives echo impulse.
 *
 * @date 2023
 * @author Aleksandar Petos
 *
 * @version [1.0 @ 06/2023] Initial version
 */
#include <stdint.h>
#include <msp430.h>

#define SMCLK               1048576
#define ACLK                32768
#define TRIGGER_PIN         BIT3            // P6.3
#define ECHO_PIN            BIT0            // P2.0
#define LED_PIN             BIT0            // P1.0
#define DISTANCE_THRESHOLD  10              // cm
#define MEASURE_INTERVAL    2048            // ~250 ms
#define BR9600              109
#define BRS9600             UCBRS_2

#define LED_PERIOD          0xffff          // 2s for ACLK, max for 16b
//#define MIN_PERIOD          (ACLK/1000)*100
/* new data read flag */
volatile uint8_t ndrf = 0;
/* variable where period is placed */
uint16_t period = 0;
uint32_t distance = 0;
uint16_t lastCount = 0;


uint16_t data = MEASURE_INTERVAL;


void triggerMeasurement() {
    // Start trigger
    P6OUT |= TRIGGER_PIN;

    // Wait a small amount of time with trigger high, > 10us required (~10 clock cycles at 1MHz MCLK)
    __delay_cycles(10);

    // End trigger
    P6OUT &= ~TRIGGER_PIN;
}

int main(void) {
    // Stop watch-dog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Configure trigger pin
    P6DIR |= TRIGGER_PIN;           // configure as output
    P6OUT &= ~TRIGGER_PIN;          // write 0

    // Initialize UART
    P4SEL |= BIT4 | BIT5;           // enable P4.4 and P4.5 for UART
    UCA1CTL1 |= UCSWRST;            // set software reset
    UCA1CTL0 = 0;                   // no parity, 8bit, 1 stop bit
    UCA1CTL1 |= UCSSEL__SMCLK;      // use SMCLK = 1 048 576 Hz
    UCA1BRW = BR9600;               // BR = 109
    UCA1MCTL |=BRS9600 + UCBRF_0;   // BRS = 2 for 9600 bps
    UCA1CTL1 &= ~UCSWRST;           // release software reset

    // Configure LED, off to start
    P1DIR |= LED_PIN;
    P1OUT &= ~LED_PIN;

    // Configure echo pin as capture input to TA1CCR1
    P2DIR &= ~ECHO_PIN;
    P2SEL |= ECHO_PIN;

    // Set up TA1 to capture in CCR1 on both edges from P2.0 (echo pin)
    TA1CCTL1 = CM_3 | CCIS_0 | SCS | CAP | CCIE;

    // Set up TA1 to compare CCR0 (measure interval)
    TA1CCR0 = MEASURE_INTERVAL;
    TA1CCTL0 = CCIE;

    // Set up TA1 with ACLK / 4 = 8192 Hz
    TA1CTL = TASSEL__ACLK | ID__4 | MC__CONTINUOUS | TACLR;

    TA0CCR0 = LED_PERIOD;
    // Output led
    TA0CCR2 = LED_PERIOD/2;         // initial state is no pulse
    TA0CCTL2 = OUTMOD_7;            // outmode is Reset/Set

    TA0CCR1 = LED_PERIOD/2;         // initial state is no pulse
    TA0CCTL1 = OUTMOD_7;            // outmode is Reset/Set

    // CCR2 and CCR1 values define the pulse width
    // CCR0 defines period of the pulse
    // Init P1.3 and P1.2 pin as alternate function
    P1SEL |= BIT3;                  // alternate function
    P1DIR |= BIT3;                  // P1.3 is TA0.2 pin
    P1SEL |= BIT2;                  // alternate function
    P1DIR |= BIT2;                  // P1.2 is TA0.1 pin
    // Activate timer
    TA0CTL = TASSEL__ACLK | MC__UP;
    __enable_interrupt();
    for (;;)
    {
        triggerMeasurement();

        // Wait for echo start
        __bis_SR_register(LPM3_bits);

        lastCount = TA1CCR1;

        // Wait for echo end
        __bis_SR_register(LPM3_bits);

        distance = TA1CCR1 - lastCount;

        distance *= 34000;

        distance >>= 14;            // division by 16384 (2 ^ 14)
        distance &= 0xff;
        UCA1TXBUF = distance;
        period = 327*distance; // 327 = LED_PERIOD/200 cm
        TA0CCR0 = period;
        TA0CCR2 = period>>1;
        TA0CCR1 = period>>1;

        if (distance <= DISTANCE_THRESHOLD)
        {
            // Turn on LED
            P1OUT |= LED_PIN;
        }
        else
        {
            // Turn off LED
            P1OUT &= ~LED_PIN;
        }

        // Wait for the next measure interval tick
//        __low_power_mode_3();
        __bis_SR_register(LPM3_bits);
    }
}

// Interrupt services will be implemented in assembler
//void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) TIMER1_A0_ISR (void)
//{
//    // Measure interval tick
//    __low_power_mode_off_on_exit();
//
//    TA1CCR0 += MEASURE_INTERVAL;
//}

//void __attribute__ ((interrupt(TIMER1_A1_VECTOR))) TIMER1_A1_ISR (void)
//{
//    // Echo pin state toggled
//    __low_power_mode_off_on_exit();
//
//    TA1IV = 0;
//}

