CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m3 -mthumb -c -g
LFLAGS = -mcpu=cortex-m3 -nostdlib -mthumb -Wl,-Map=main.map -T main.ld

all : startup.o main.o
	$(CC) $(LFLAGS) -o main.elf main.o startup.o 

startup : startup.c
	$(CC) $(CFLAGS) startup.c

main : main.c
	$(CC) $(CFLAGS) main.c
	
clean:
	rm -f *.o *.elf *.map

debug:
	qemu-system-arm \
  -M lm3s6965evb \
  -kernel main.elf \
  -gdb tcp::1234 \
  -S \
  -nographic

gdb:
	gdb-multiarch main.elf -ex "target remote localhost:1234" -ex "layout src" -ex "layout regs"

	

