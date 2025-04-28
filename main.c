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

    // Reserve space for the initial stack frame (8 registers: R0-R3, R12, LR, PC, xPSR)
    sp -= 8;

    sp[7] = 0x01000000;         // xPSR (Thumb bit set)
    sp[6] = (uint32_t)task_func; // PC (Task entry point)
    sp[5] = 0xFFFFFFFD;          // LR (return to Thread mode using PSP)
    sp[4] = 0;                   // R12
    sp[3] = 0;                   // R3
    sp[2] = 0;                   // R2
    sp[1] = 0;                   // R1
    sp[0] = 0;                   // R0

    *stack_addr = sp; // Save the new stack pointer
}

void SysTick_Handler(void) {
    g_tick_cntr++;
    if (g_tick_cntr % 10 == 0) { // Every 10ms
        ICSR |= (1 << 28); // Trigger PendSV
    }
}

// PendSV Handler (minimal context switch)
__attribute__((naked)) void PendSV_Handler(void) {
    __asm volatile(
        // Get current PSP (points to hardware-saved stack frame)
        "MRS r0, psp\n"

        // Save current task's stack pointer
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
        // Restore PSP (hardware will handle the rest during exception return)
        "MSR psp, r0\n"
        "BX lr\n"
    );
}

void init_systick(void) {
    SYSTICK_LOAD = 8000 - 1;  // 1ms tick (assuming 8 MHz clock)
    SYSTICK_VAL = 0;
    SYSTICK_CTRL = 0x07;      // Enable SysTick, use system clock, enable interrupt
}

int main(void) {
    task0_sp = task0_stack;
    task1_sp = task1_stack;

    // Initialize task stacks
    init_task_stack(&task0_sp, task0);
    init_task_stack(&task1_sp, task1);

    // Initialize SysTick
    init_systick();

    // Set SysTick priority (e.g., 0x80, medium priority)
    SHPR2 |= (0x80 << 24); // Bits 31:24 for SysTick

    // Set PendSV priority (e.g., 0xFF, lowest priority)
    SHPR3 |= (0xFF << 16); // Bits 23:16 for PendSV

    // Start first task
    __asm volatile(
        "LDR r0, =task0_sp\n"
        "LDR r0, [r0]\n"
        "MSR psp, r0\n"           // Set PSP to task0 stack
        "MOVS r0, #2\n"           // CONTROL = 0x2 -> Use PSP, unprivileged mode
        "MSR CONTROL, r0\n"
        "ISB\n"
    );

    // Trigger PendSV manually to start task switching
    ICSR |= (1 << 28);

    while (1) {
        // Main loop idle
    }
}

void HardFault_Handler(void) {
  __asm volatile(
    "BKPT #0\n" // Break into debugger
  );
  }
