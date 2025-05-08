/* Linker script for LM3S6965EVB (Cortex-M3) */

/* Define memory regions */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K  /* Flash memory: 256 KB */
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K   /* SRAM: 64 KB */
}

/* Define stack top (end of SRAM) */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

/* Define sections */
SECTIONS
{
    /* Interrupt vector table in flash */
    .vector :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))  /* Vector table, retained even if unused */
    } > FLASH

    /* Program code and read-only data in flash */
    .text :
    {
        . = ALIGN(4);
        *(.text*)          /* All code sections */
        *(.rodata*)        /* Read-only data */
        . = ALIGN(4);
        _etext = .;        /* End of code in flash */
    } > FLASH

    /* Initialized data: stored in flash, copied to SRAM at startup */
    .data : AT(_etext)
    {
        . = ALIGN(4);
        _sdata = .;        /* Start of data in SRAM */
        *(.data*)          /* Initialized data */
        . = ALIGN(4);
        _edata = .;        /* End of data in SRAM */
    } > SRAM

    /* Load address of .data in flash */
    _sidata = LOADADDR(.data);

    /* Uninitialized data (BSS) in SRAM */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* Start of BSS */
        *(.bss*)           /* Uninitialized data */
        *(COMMON)          /* Common uninitialized data */
        . = ALIGN(4);
        _ebss = .;         /* End of BSS */
    } > SRAM

    /* Discard unused sections */
    /DISCARD/ :
    {
        *(.ARM.exidx*)     /* Exception unwinding (not needed for Cortex-M3) */
        *(.comment)        /* Comments */
        *(.note*)          /* Notes */
    }
}