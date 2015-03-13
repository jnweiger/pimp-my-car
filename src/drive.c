/*
 * drive.c -- controller for an RC-car
 *
 * Copyright (C) 2011, jw@suse.de, distribute under GPL, use with mercy.
 *
 * V0.2		Seperated poll_rx() from update(). Introduced CMD_*
 * 		Added a 100hz timer. 
 * 		added right_right_manover(): U-turn in 13 moves.
 *
 */
#include "config.h"
#include "cpu_mhz.h"

#include <util/delay.h>			// needs F_CPU from cpu_mhz.h
// #include <util/atomic.h>		// ATOMIC_BLOCK violates c99 -> compile time error.
#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t tick100 = 0;

uint8_t poll_rx();
void do_rx_cmd(uint8_t cmd);
void motors(uint8_t cmd);
void timer2_100hz();

void right_right_manover(uint8_t cmd)
{
  if (cmd == CMD_BWD)
    {
      uint8_t i;
      // seven-point turn	(Wenden in (mehr als) drei Zügen)
      for (i = 0; i < 7; i++)
        {
	  cmd = poll_rx();
	  if (cmd != CMD_IDLE && cmd != CMD_BWD) return;	// Abort manover

	  if (i & 0x01)
	    {
	      motors(CMD_FWD|CMD_RIGHT); _delay_ms(550.0); 
	      motors(CMD_RIGHT);          _delay_ms(200.0);
	      motors(CMD_LEFT);           _delay_ms( 50.0);
	    }
	  else
	    {
	      motors(CMD_BWD|CMD_LEFT);  
	      if (i == 0)
	      				 _delay_ms(300.0); 
	      else
	        			 _delay_ms(650.0); 
	      motors(CMD_LEFT);          _delay_ms(200.0);
	      motors(CMD_RIGHT);         _delay_ms( 50.0);
	    }
	}
    }
  else if (cmd == CMD_FWD)
    {
      uint16_t i;
      // honk, using the drive motor.
      for (i = 0; i < 1020; i++)
        {
	  cmd = poll_rx();
	  if (cmd != CMD_IDLE && cmd != CMD_FWD) return;	// Abort manover

	  motors(CMD_FWD);
	  _delay_us(400.0);
	  motors(CMD_IDLE);
	  _delay_ms((i&0x100) ? 0.7 : 0.9);
	}
    }
}

// from: file:///opt/cross/avr/share/doc/avr-libc-1.7.1/user-manual/optimization.html
#define atomic_cli()	__asm volatile( "cli" ::: "memory" )
#define atomic_sei()	__asm volatile( "sei" ::: "memory" )

static inline uint16_t get_tick100()
{
  uint16_t r;
  atomic_cli();
  r = tick100;
  atomic_sei();
  return r;
}

int main()
{
  // configure all RX_* pins as input:
  P_DDR(RX_LEFT)  &= ~P_BIT(RX_LEFT);
  P_DDR(RX_RIGHT) &= ~P_BIT(RX_RIGHT);
  P_DDR(RX_BWD)   &= ~P_BIT(RX_BWD);
  P_DDR(RX_FWD)   &= ~P_BIT(RX_FWD);
  P_DDR(RX_TURBO) &= ~P_BIT(RX_TURBO);
  P_DDR(RX_ENC)   &= ~P_BIT(RX_ENC);

  // configure all MOT_* pins as output:
  P_DDR(MOT_LEFT)  |= P_BIT(MOT_LEFT);
  P_DDR(MOT_RIGHT) |= P_BIT(MOT_RIGHT);
  P_DDR(MOT_BWD)   |= P_BIT(MOT_BWD);
  P_DDR(MOT_FWD)   |= P_BIT(MOT_FWD);
  P_DDR(LED1)      |= P_BIT(LED1);

  timer2_100hz();

  // greetings ...
  if (MCUSR == (1<<PORF)) _delay_ms(1000.0); 	// delay, only if from power on, please.
  MCUSR = 0;
  P_PORT(MOT_LEFT) |= P_BIT(MOT_LEFT);
  _delay_ms(200.0); 
  P_PORT(MOT_LEFT) &= ~P_BIT(MOT_LEFT);
  _delay_ms(400.0); 
  P_PORT(MOT_LEFT) |= P_BIT(MOT_LEFT);
  _delay_ms(200.0); 
  P_PORT(MOT_LEFT) &= ~P_BIT(MOT_LEFT);

  for (;;)
    {
      uint8_t cmd = poll_rx();
      do_rx_cmd(cmd);
    }
}

void motors(uint8_t cmd)
{
  // steering
  if (cmd & CMD_LEFT)
    {
      P_PORT(MOT_RIGHT) &= ~P_BIT(MOT_RIGHT);
      P_PORT(MOT_LEFT)  |=  P_BIT(MOT_LEFT);
    }
  else if (cmd & CMD_RIGHT)
    {
      P_PORT(MOT_LEFT)  &= ~P_BIT(MOT_LEFT);
      P_PORT(MOT_RIGHT) |=  P_BIT(MOT_RIGHT);
    }
  else
    {
      P_PORT(MOT_RIGHT) &= ~P_BIT(MOT_RIGHT);
      P_PORT(MOT_LEFT)  &= ~P_BIT(MOT_LEFT);
    }

  // driving
  if (cmd & CMD_FWD)
    {
      P_PORT(MOT_BWD) &= ~P_BIT(MOT_BWD);
      P_PORT(MOT_FWD) |=  P_BIT(MOT_FWD);
    }
  else if (cmd & CMD_BWD)
    {
      P_PORT(MOT_FWD) &= ~P_BIT(MOT_FWD);
      P_PORT(MOT_BWD) |=  P_BIT(MOT_BWD);
    }
  else
    {
      P_PORT(MOT_BWD) &= ~P_BIT(MOT_BWD);
      P_PORT(MOT_FWD) &= ~P_BIT(MOT_FWD);
    }
}

void do_rx_cmd(uint8_t cmd)
{
  static uint8_t c1 = CMD_IDLE;
  static uint8_t c2 = CMD_IDLE;
  static uint8_t do_shift = 0;

  if (!cmd) do_shift = 1;
  if (cmd && do_shift)
    {
      do_shift = 0;
      if (c1 == CMD_RIGHT && c2 == CMD_RIGHT)
	{
	  right_right_manover(cmd);
          c2 = c1 = CMD_IDLE;
	  return;
	}
      c2 = c1; c1 = cmd;
    }
  motors(cmd);
}

uint8_t poll_rx()
{

  uint8_t cmd = 0;
  // steering
  if (P_PIN(RX_LEFT) & P_BIT(RX_LEFT))
    {
      cmd = CMD_LEFT;
    }
  else if (P_PIN(RX_RIGHT) & P_BIT(RX_RIGHT))
    {
      cmd = CMD_RIGHT;
    }

  // driving
  if (P_PIN(RX_FWD) & P_BIT(RX_FWD))
    {
      cmd |= CMD_FWD;
    }
  else if (P_PIN(RX_BWD) & P_BIT(RX_BWD))
    {
      cmd |= CMD_BWD;
    }
  return cmd;
}

void timer2_100hz()
{
  CLKPR = 0x80;		// CLKPCE open gate for 4 cycles
  CLKPR = 0x03;		// same effect as CKDIV8 fuse.
  PRR = 0;		// PRTIM2 bit = 0;	// to enable Timer/Counter2 module

  TCCR2A = (1<<WGM21)|(0<<WGM20);		// 10:  CTC mode
  TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);	// clk prescaper 1024
  OCR2A = 100;					// 100msec per interrupt
  TIMSK2 |= (1<<OCIE2A);			// enable SIG_OUTPUT_COMPARE2
  // I am always unsure, if CTC triggers a compare or an overflow.
  sei();
}

SIGNAL(SIG_OUTPUT_COMPARE2A) // uninterruptable
{
  P_PIN(LED1) |= P_BIT(LED1);        // toggle
  tick100++;
}
