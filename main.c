#include <stdio.h>
#include <stdio.h>

#define SYSTICK_CTRL (*(volatile uint32_t*)0xE000E010)
#define SYSTICK_LOAD (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL  (*(volatile uint32_t*)0xE000E018)

volatile uint32_t g_tick_cntr = -1;

void SysTick_Handler(void)
{
  g_tick_cntr++;
}

void init_systick(void)
{
  SYSTICK_LOAD = 8000 -1;
  SYSTICK_VAL = 0;
  SYSTICK_CTRL = 7U;
}

int main(void)
{
  init_systick();

  while(1)
  {
    __asm volatile ("wfi");
  }
  return 0;
}