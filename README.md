# TSC-8 -- Tori's, uhh, "Silly" Computer

The **TSC-8** is an experimental 8-bit computer based on the WDC 65C02 microprocessor. The project originates from a desire to own an Altair 8800 - which are *far* too expensive. Not satisfied with the various emulators and kit computers I decided to design and build my own 8-bit microcomputer.

I wanted it to be built using all modern, off-the-shelf parts, so for the CPU I chose the ever-present 6502 architecture. This was partially just down to familiarity - I have programmed the Commodore 64 in the past, and I do not have any experience programming a Z80.


This project is currently in a design phase and has no physical prototypes. The plan is to design everything, then build a prototype on breadboards. After that, I will move on to a PCB prototype before (maybe) releasing it as a product. The design will always be open source. You can find any updates on the project over at [my site](https://toriii.dev/projects/tsc8).

## In this repository
This repository currently contains everything you need to run the initial design in Logisim Evolution. I am not too sure how Logisim Evolution handles JAR plugins in .circ files, so you may need to download and load the [logi6502](https://github.com/SolraBizna/logi6502) plugin.


The `rom.bin` file is assembled using `cc65`. You must install this before trying to compile the ROM yourself.
