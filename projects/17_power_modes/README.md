# Power Modes

*For reference: microcontroller's datasheet page 228.*

There are four levels of operation for the microcontroller defined as:
- Run mode
- Sleep mode
- Deep-Sleep mode
- Hibernate mode

## Run Mode
In Run mode, the microcontroller actively executes code. Run mode provides normal operation of
the processor and all of the peripherals that are currently enabled by the peripheral-specific RCGC
registers. The system clock can be any of the available clock sources including the PLL.

## Sleep Mode
In Sleep mode, the clock frequency of the active peripherals is unchanged, but the processor and
the memory subsystem are not clocked and therefore no longer execute code. Sleep mode is entered
by the Cortex-M4F core executing a WFI (Wait for Interrupt) instruction. Any properly configured
interrupt event in the system brings the processor back into Run mode.
Additional sleep modes are available that lower the power consumption of the SRAM and Flash
memory. However, the lower power consumption modes have slower sleep and wake-up times.

## Deep-Sleep Mode
In Deep-Sleep mode, the clock frequency of the active peripherals may change (depending on the
Deep-Sleep mode clock configuration) in addition to the processor clock being stopped.
Deep-Sleep mode is entered by first setting the SLEEPDEEP bit in the
System Control (SYSCTRL) register (see page 166) and then executing a WFI instruction. Any
properly configured interrupt event in the system brings the processor back into Run mode.
If the PLL is running at the time of the WFI instruction, hardware powers the PLL down.
Additional sleep modes are available that lower the power consumption of the SRAM and Flash
memory. However, the lower power consumption modes have slower sleep and wake-up times.

## Hibernate Mode
In this mode, the power supplies are turned off to the main part of the microcontroller and only the
Hibernation module's circuitry is active. An external wake event or RTC event is required to bring
the microcontroller back to Run mode. The Cortex-M4F processor and peripherals outside of the
Hibernation module see a normal "power on" sequence and the processor starts running code.
Software can determine if the microcontroller has been restarted from Hibernate mode by inspecting
the Hibernation module registers.
The Hibernation module can be independently supplied from an external battery or an auxiliary power supply.

## Circuit

<img src="./circuit.svg" width=70% height=70%>
