#include <stdint.h>

#define SYSTICK_CTRL (*(volatile uint32_t*)0xE000E010)
#define SYSTICK_LOAD (*(volatile uint32_t*)0xE000E014)
#define SYSTICK_VAL  (*(volatile uint32_t*)0xE000E018)
#define ICSR         (*(volatile uint32_t*)0xE000ED04)
#define SHPR2        (*(volatile uint32_t*)0xE000ED20) // SysTick priority
#define SHPR3        (*(volatile uint32_t*)0xE000ED24) // PendSV priority

volatile uint32_t g_var1 = -1;
volatile uint32_t g_var2 = -1;
volatile uint32_t g_tick_cntr = 0;

// Debug variables for HardFault
volatile uint32_t fault_hfsr = 0;
volatile uint32_t fault_cfsr = 0;
volatile uint32_t fault_mmfar = 0;
volatile uint32_t fault_bfar = 0;

uint32_t *task0_sp;
uint32_t *task1_sp;

// Ensure stack arrays are 8-byte aligned
__attribute__((aligned(8))) uint32_t task0_stack[64];
__attribute__((aligned(8))) uint32_t task1_stack[64];

volatile uint32_t current_task = 0;

// Simple tasks
void task0(void) {
    while (1) {
        g_var1++;
    }
}

void task1(void) {
    while (1) {
        g_var2++;
    }
}

// Initialize a task stack with proper alignment
void init_task_stack(uint32_t **stack_addr, void (*task_func)(void)) {
    uint32_t *sp = *stack_addr + 64; // Point to the end of the stack
    
    // Ensure 8-byte alignment
    sp = (uint32_t*)((uint32_t)sp & ~0x7);
    // Reserve space for full exception frame (16 registers: R0-R3, R12, LR, PC, xPSR, R4-R11)
    sp -= 16;

    // Hardware-saved registers
    sp[15] = 0x01000000;         // xPSR (Thumb bit set)
    sp[14] = ((uint32_t)task_func) | 1; // PC (Task entry point)
    sp[13] = 0xFFFFFFFD;          // LR (return to Thread mode using PSP)
    sp[12] = 0;                   // R12
    sp[11] = 0;                   // R3
    sp[10] = 0;                   // R2
    sp[9]  = 0;                   // R1
    sp[8]  = 0;                   // R0
    // Manually saved registers (R4-R11)
    sp[7]  = 0;                   // R11
    sp[6]  = 0;                   // R10
    sp[5]  = 0;                   // R9
    sp[4]  = 0;                   // R8
    sp[3]  = 0;                   // R7
    sp[2]  = 0;                   // R6
    sp[1]  = 0;                   // R5
    sp[0]  = 0;                   // R4

    *stack_addr = sp; // Save the new stack pointer
}

void SysTick_Handler(void) {
    g_tick_cntr++;
    if (g_tick_cntr % 1 == 0) { // Every 1ms
        ICSR |= (1 << 28); // Trigger PendSV
    }
}

// PendSV Handler (minimal context switch)
__attribute__((naked)) void PendSV_Handler(void) {
    __asm volatile(
        "MRS r0, psp\n"             // Get current PSP
        "BIC r0, r0, #7\n"         // Ensure 8-byte alignment
        "STMDB r0!, {r4-r11}\n"     // Save high registers
        "LDR r1, =current_task\n"
        "LDR r2, [r1]\n"
        "CMP r2, #0\n"
        "BEQ save_task0\n"

        // Save task1_sp
        "LDR r3, =task1_sp\n"
        "STR r0, [r3]\n"
        "B load_task\n"

        "save_task0:\n"
        // Save task0_sp
        "LDR r3, =task0_sp\n"
        "STR r0, [r3]\n"

        "load_task:\n"
        // Toggle task
        "LDR r1, =current_task\n"
        "LDR r2, [r1]\n"
        "EOR r2, r2, #1\n"
        "STR r2, [r1]\n"

        "CMP r2, #0\n"

        "BEQ load_task0\n"

        // Load task1_sp
        "LDR r3, =task1_sp\n"
        "LDR r0, [r3]\n"
        "B restore\n"

        "load_task0:\n"
        // Load task0_sp
        "LDR r3, =task0_sp\n"
        "LDR r0, [r3]\n"

        "restore:\n"
        "LDMIA r0!, {r4-r11}\n"     // Restore high registers
        "MSR psp, r0\n"             // Restore PSP
        "BX lr\n"                   // Return from exception
    );
}

void init_systick(void) {
    SYSTICK_LOAD = 8000 - 1;  // 1ms tick (assuming 8 MHz clock)
    SYSTICK_VAL = 0;
    SYSTICK_CTRL = 0x07;      // Enable SysTick, use system clock, enable interrupt
}

int main(void) {
    // Disable interrupts during setup
    __asm volatile("CPSID i\n");

    task0_sp = task0_stack;
    task1_sp = task1_stack;

    // Initialize task stacks
    init_task_stack(&task0_sp, task0);
    init_task_stack(&task1_sp, task1);

    // Initialize SysTick
    init_systick();

    // Set SysTick priority (level 2)
    SHPR2 |= (2 << 30);

    // Set PendSV priority (level 3)
    SHPR3 |= (3 << 22);

    // Start first task
    __asm volatile(
        "LDR r0, =task0_sp\n"
        "LDR r0, [r0]\n"
        "BIC r0, r0, #7\n"         // Ensure 8-byte alignment
        "MSR psp, r0\n"            // Set PSP to task0 stack
        "MOVS r0, #2\n"            // CONTROL = 0x2 -> Use PSP, unprivileged mode
        "MSR CONTROL, r0\n"
        "ISB\n"
        "CPSIE i\n"                // Re-enable interrupts
        "LDR r0, =task0\n"
        "BX r0\n"                  // Jump to task0
    );

    while (1) {
        // Main loop idle
    }
}

void HardFault_Handler(void) {
    fault_hfsr = *(volatile uint32_t*)0xE000ED2C; // HFSR
    fault_cfsr = *(volatile uint32_t*)0xE000ED28; // CFSR
    fault_mmfar = *(volatile uint32_t*)0xE000ED34; // MMFAR
    fault_bfar = *(volatile uint32_t*)0xE000ED38; // BFAR
    __asm volatile("BKPT #0\n");
    while (1);
}