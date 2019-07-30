# Flashing Samples

At the moment I'm flashing one wav to the board and then setting the start of each sample as a magic number.

`st-flash write 808.wav 0x8010000`

In the future we should make it so that you can flash an array of samples, plus some meta data that contains sample start, length, sampling freq?, file format? (add decoder? might have to be external codec)

E.g.

0x8000000
    
    struct samples[8]

0x8001000
    
    samples begin here


Also could consider using the external flash, or sd card? and then copying samples to flash when they're chosen

SD card would require handing FATFS - but could be very useful!
http://elm-chan.org/fsw/ff/00index_e.html

# Building

Dependencies should all be in git - might be better to remove the SDK / trim down other targets

`./make`

# Flashing

Debug symbols are on so easiest is st-util and gdb

* Open two terminals

* In one run `st-util`

* In the other run `arm-none-eabi-gdb`
    * The .gdbinit file in the repo should handle loading and setting a breakpoint at main()
    * Enter c to start the program
    * If you edit code: CTRL-C, make, load. Will load the new debug symbols and program
