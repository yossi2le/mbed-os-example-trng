# mbed-os-example-trng

TRNG test example for Mbed OS

## Getting started with TRNG test example ##

This is a test example of an application that uses the TRNG APIs.

TRNG is true random number generator and is supported by numorous devices.

The application invokes the TRNG APIs and prints the results after each such invocation. 

## Required hardware
* An [FRDM-K64F](http://os.mbed.com/platforms/FRDM-K64F/) development board.
* An SD card.
* A micro-USB cable.

### Other hardware

Although the board shown in this examples are K64F, the example should work on any device that support the trng feature.

##  Getting started ##

 1. Import the example.

    ```
    mbed import mbed-os-example-trng
    cd mbed-os-example-trng
    ```

 2. Deploy mbed-os

    ```
    mbed deploy
    ```

 3. Compile and run test.

    For example, for `K64F` with `GCC`:

    ```
    mbed test -t GCC_ARM -m K64F -n ************* test-basic-trng **************
    ```

 4. The test should run and display a series of results following the StoragLite API invocations.

## Troubleshooting

If you have problems, you can review the [documentation](https://os.mbed.com/docs/latest/tutorials/debugging.html) for suggestions on what could be wrong and how to fix it.
