
# README

## Download all the necessary files and tools:
```bash
sudo apt-get update && sudo apt-get install -y \
    gcc-arm-none-eabi \
    gdb-multiarch \
    qemu-system-arm \
    make \
    build-essential
```

## Run the below command in terminal 1 this will start the gdb server at port 1234:

```bash
qemu-system-arm \
  -M lm3s6965evb \
  -kernel main.elf \
  -gdb tcp::1234 \
  -S \
  -nographic
```

    - **`-M lm3s6965evb`**: Specifies the TI lm3s6965evb target (you can replace this with any supported ARM model).
    - **`-kernel main.elf`**: Points to your compiled ELF file.
    - **`-gdb tcp::1234`**: Opens a GDB server on TCP port 1234.
    - **`-S`**: Pauses execution immediately for GDB to connect.
    - **`-nographic`**: Disables graphical output and logs to the terminal.


## To start debug the code run this below command in terminal 2:
```bash
gdb-multiarch main.elf
```

## Below command will establish a connnection, reset, halt and put a brake point at main:
```bash
target remote localhost:1234
monitor reset halt
break main
```

### Sample debug commands:
```bash
info register   ----> shows the GPRs

x/i $pc      ---> examine instruction at PC

x/1dw <address>   -->(1, read one count, d - decimal , w - word length, <address> eg: 0x2000000)

run

next   --> next c line (AKA step over)

nexti  --> next instruction 

step --> step in 

stepi  --> step into instruction

continue --> free run

tui enable   --> gui for src code

tui reg all  --> show all the registers

layout reg
 
layout split 

```

## For more gui with python you can refer the below link:
 
```bash
https://www.kitploit.com/2016/01/gdb-dashboard-modular-visual-interface.html

Download the .gdbinit file using:

wget -P ~ git.io/.gdbinit
```
