/****************************************************************************
 * arch/arm/src/stm32/stm32_timerisr.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <time.h>
#include <debug.h>
#include <nuttx/arch.h>
#include <nuttx/timers/arch_timer.h>
#include <arch/board/board.h>

#include "nvic.h"
#include "clock/clock.h"
#include "arm_internal.h"
#include "systick.h"

#include "chip.h"
#include "stm32.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* The desired timer interrupt frequency is provided by the definition
 * CLK_TCK (see include/time.h).  CLK_TCK defines the desired number of
 * system clock ticks per second.  That value is a user configurable setting
 * that defaults to 100 (100 ticks per second = 10 MS interval).
 *
 * The RCC feeds the Cortex System Timer (SysTick) with the AHB clock (HCLK)
 * divided by 8.  The SysTick can work either with this clock or with the
 * Cortex clock (HCLK), configurable in the SysTick Control and Status
 * register.
 */

#undef CONFIG_STM32_SYSTICK_HCLKd8 /* Power up default is HCLK, not HCLK/8 */
                                   /* And I don't know now to re-configure it yet */

#ifdef CONFIG_STM32_SYSTICK_HCLKd8
#  define SYSTICK_CLOCK  (STM32_HCLK_FREQUENCY / 8)
#else
#  define SYSTICK_CLOCK  (STM32_HCLK_FREQUENCY)
#endif

#define SYSTICK_RELOAD ((SYSTICK_CLOCK / CLK_TCK) - 1)

/* The size of the reload field is 24 bits.  Verify that the reload value
 * will fit in the reload register.
 */

#define SYSTICK_MAX 0x00ffffff
#if SYSTICK_RELOAD > SYSTICK_MAX
#  error SYSTICK_RELOAD exceeds the range of the RELOAD register
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Function:  stm32_timerisr
 *
 * Description:
 *   The timer ISR will perform a variety of services for various portions
 *   of the systems.
 *
 ****************************************************************************/

#if !defined(CONFIG_ARMV7M_SYSTICK) && !defined(CONFIG_TIMER_ARCH)
static int stm32_timerisr(int irq, uint32_t *regs, void *arg)
{
  /* Process timer interrupt */

  nxsched_process_timer();
  return 0;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_CLOCK_ADJTIME

/****************************************************************************
 * Function:  up_adj_timer_period
 *
 * Description:
 *   Adjusts timer period. This call is used when adjusting timer period as
 *   defined in adjtime() function.
 *
 * Input Parameters:
 *   period_inc_usec  - period adjustment in usec (reset to default value
 *                      if 0)
 *
 ****************************************************************************/

void up_adj_timer_period(long long period_inc_usec)
{
  uint32_t period;
  long long period_inc;

  if (period_inc_usec == 0)
    {
      period_inc = 0;
    }
  else
    {
      period_inc = (SYSTICK_CLOCK / 1000000) * period_inc_usec - 1;
    }

  period = SYSTICK_RELOAD + period_inc;

  /* Check whether period is at maximum value. */

  if (period > SYSTICK_MAX)
    {
      period = SYSTICK_MAX;
    }

  putreg32(period, NVIC_SYSTICK_RELOAD);
}

/****************************************************************************
 * Function:  up_get_timer_period
 *
 * Description:
 *   This function returns the timer period in usec.
 *
 * Input Parameters:
 *   period_usec  - returned timer period in usec
 *
 ****************************************************************************/

void up_get_timer_period(long long *period_usec)
{
  uint32_t period;

  period = getreg32(NVIC_SYSTICK_RELOAD);

  *period_usec = ((period + 1) / (SYSTICK_CLOCK / 1000000));
}

#endif

/****************************************************************************
 * Function:  up_timer_initialize
 *
 * Description:
 *   This function is called during start-up to initialize
 *   the timer interrupt.
 *
 ****************************************************************************/

void up_timer_initialize(void)
{
  uint32_t regval;

  /* Set the SysTick interrupt to the default priority */

  regval = getreg32(NVIC_SYSH12_15_PRIORITY);
  regval &= ~NVIC_SYSH_PRIORITY_PR15_MASK;
  regval |= (NVIC_SYSH_PRIORITY_DEFAULT << NVIC_SYSH_PRIORITY_PR15_SHIFT);
  putreg32(regval, NVIC_SYSH12_15_PRIORITY);

  /* Make sure that the SYSTICK clock source is set correctly */

#if 0 /* Does not work.  Comes up with HCLK source and I can't change it */
  regval = getreg32(NVIC_SYSTICK_CTRL);
#ifdef CONFIG_STM32_SYSTICK_HCLKd8
  regval &= ~NVIC_SYSTICK_CTRL_CLKSOURCE;
#else
  regval |= NVIC_SYSTICK_CTRL_CLKSOURCE;
#endif
  putreg32(regval, NVIC_SYSTICK_CTRL);
#endif

#if defined(CONFIG_ARMV7M_SYSTICK) && defined(CONFIG_TIMER_ARCH)
  up_timer_set_lowerhalf(systick_initialize(true, STM32_HCLK_FREQUENCY, -1));
#else
  /* Configure SysTick to interrupt at the requested rate */

  putreg32(SYSTICK_RELOAD, NVIC_SYSTICK_RELOAD);

  /* Attach the timer interrupt vector */

  irq_attach(STM32_IRQ_SYSTICK, (xcpt_t)stm32_timerisr, NULL);

  /* Enable SysTick interrupts */

  putreg32((NVIC_SYSTICK_CTRL_CLKSOURCE | NVIC_SYSTICK_CTRL_TICKINT |
            NVIC_SYSTICK_CTRL_ENABLE), NVIC_SYSTICK_CTRL);

  /* And enable the timer interrupt */

  up_enable_irq(STM32_IRQ_SYSTICK);
#endif
}
