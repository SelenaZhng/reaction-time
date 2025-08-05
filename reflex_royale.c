#include "led.h"
#include "3140_concur.h"
#include "realtime.h"
#include "lcd.h"
#include <stdlib.h>

#define RT_STACK 20

static const int SWITCH_1_PIN = 3;   // SW1
static const int SWITCH_3_PIN = 12;  // SW3

//LED color states
typedef enum { LED_GREEN = 0, LED_RED  = 1 } led_color_t;

//scores for each player
static volatile int seg1_count = 0;
static volatile int seg4_count = 0;

//which led was last lit
static volatile led_color_t last_led   = LED_GREEN;

//prevents multiple presses for single LED flash
static volatile int press_handled      = 1;

//stops when someone reaches over 5 points
static volatile int game_over          = 0;

extern volatile realtime_t current_time;

//randomly flashes either red or green LED after a random delay
void pBlink(void) {
    while (1) {
        if (game_over) {
            // stop blinking when game ends
            while (1) process_blocked();
        }

        int delay_ms = 2000 + rand() % 6001;
        int pick = rand() % 2;

        realtime_t target = current_time;
        target.msec += delay_ms;
        target.sec  += target.msec / 1000;
        target.msec %= 1000;

        while (!(target.sec < current_time.sec ||
                 (target.sec == current_time.sec && target.msec <= current_time.msec))) {
            process_blocked();
        }

        PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_ISF(1);
        PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_ISF(1);

        // record which LED flashed and re-enable press handling
        last_led = pick ? LED_RED : LED_GREEN;
        press_handled = 0;

        // flash LED
        if (last_led == LED_GREEN) {
            green_on_frdm();
            delay(200);
            green_off_frdm();
        } else {
            red_on_frdm();
            delay(200);
            red_off_frdm();
        }
    }
}

int main(void) {
    //initialize LCD and LED
    init_lcd();
    led_init();

    // initial display: seg1=0, seg4=0
    displayDigit(1, 0);
    displayDigit(4, 0);

    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

    //configure SW1
    PORTC->PCR[SWITCH_1_PIN] = PORT_PCR_MUX(1)
                             | PORT_PCR_PE_MASK
                             | PORT_PCR_PS_MASK
                             | PORT_PCR_IRQC(0xA);
    PTC->PDDR &= ~(1 << SWITCH_1_PIN);

    // configure SW3
    PORTC->PCR[SWITCH_3_PIN] = PORT_PCR_MUX(1)
                             | PORT_PCR_PE_MASK
                             | PORT_PCR_PS_MASK
                             | PORT_PCR_IRQC(0xA);
    PTC->PDDR &= ~(1 << SWITCH_3_PIN);

    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
    __enable_irq();

    if (process_rt_create(pBlink, RT_STACK,
                          &(realtime_t){0,0}, &(realtime_t){10,0}) < 0) {
        return -1;
    }
    process_start();

    while (1) {
        process_blocked();
    }
    return 0;
}

void PORTC_PORTD_IRQHandler(void) {
    if (game_over) {
        // game ended: no further input or blinking
        return;
    }

    uint32_t flags = PORTC->ISFR;

    // SW1 pressed: add 1 point if green, -1 if red
    if (!press_handled && (flags & (1 << SWITCH_1_PIN))) {
        if (last_led == LED_GREEN) {
            if (seg4_count < 5) seg4_count++;
        }
        else {
            if (seg4_count > 0) seg4_count--;
        }
        displayDigit(4, seg4_count);
        press_handled = 1;

        if (seg4_count == 5) {
            game_over = 1;
            clearDigit(1);
            clearDigit(4);
            turnOnSegment(2, 'A');
            turnOnSegment(2, 'B');
            turnOnSegment(2, 'F');
            turnOnSegment(2, 'G');
            turnOnSegment(2, 'E');
            displayDigit(3, 1);
        }
    }

    if (flags & (1 << SWITCH_1_PIN)) {
        PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_ISF(1);
    }

    // SW3 pressed: add 1 point if green, -1 if red
    if (!press_handled && (flags & (1 << SWITCH_3_PIN))) {
        if (last_led == LED_GREEN) {
            if (seg1_count < 5) seg1_count++;
        }
        else {
            if (seg1_count > 0) seg1_count--;
        }

        displayDigit(1, seg1_count);
        press_handled = 1;

        if (seg1_count == 5) {
            game_over = 1;
            clearDigit(1);
            clearDigit(4);
            turnOnSegment(2, 'A');
            turnOnSegment(2, 'B');
            turnOnSegment(2, 'F');
            turnOnSegment(2, 'G');
            turnOnSegment(2, 'E');
            displayDigit(3, 2);
        }
    }

    if (flags & (1 << SWITCH_3_PIN)) {
        PORTC->PCR[SWITCH_3_PIN] |= PORT_PCR_ISF(1);
    }
}
