#include <stdint.h>

/* Symbols from linker script */
extern uint32_t _sStack;
extern uint32_t _startflashdataaddr, _sdatasram, _edatasram;
extern uint32_t _sbss, _ebss;

/* Forward declarations */
int main(void);
void Reset_Handler(void);
void Default_Handler(void);

/* Weak aliases for interrupt handlers to Default_Handler */
void NMI_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)      __attribute__((weak, alias("Default_Handler")));

/* Vector table */
__attribute__((section(".vectors")))
void (* const vector_table[])(void) = {
    (void (*)(void))(&_sStack),     // Initial Stack Pointer
    Reset_Handler,                  // Reset
    NMI_Handler,                    // NMI
    HardFault_Handler,              // HardFault
    MemManage_Handler,             // MemManage
    BusFault_Handler,              // BusFault
    UsageFault_Handler,            // UsageFault
    0, 0, 0, 0,                     // Reserved
    SVC_Handler,                   // SVCall
    DebugMon_Handler,             // Debug Monitor
    0,                              // Reserved
    PendSV_Handler,               // PendSV
    SysTick_Handler,              // SysTick
    // IRQ0 to IRQ31 — fill with Default_Handler
    [16 ... 47] = Default_Handler
};

/* Reset Handler */
void Reset_Handler(void)
{
    // Copy initialized data from flash to SRAM
    uint32_t *src = &_startflashdataaddr;
    uint32_t *dest = &_sdatasram;
    while (dest < &_edatasram)
    {
        *dest++ = *src++;
    }

    // Zero initialize the .bss section
    dest = &_sbss;
    while (dest < &_ebss)
    {
        *dest++ = 0U;
    }

    // Call the application's entry point
    main();

    // If main returns, loop forever
    while (1);
}

/* Default Handler */
void Default_Handler(void)
{
    __asm volatile(
        "BKPT #0\n" // Break into debugger
      );
    while (1);
}
