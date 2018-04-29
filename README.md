# mbed-os-example-trng

TRNG test example for Mbed OS

## Getting started with TRNG test example ##

This is a test example of an application that uses the TRNG APIs.

The application invokes the TRNG APIs and prints the results after each such invocation, the test is composed out of two parts: the first, generate a trng buffer and try to compress it, if compressession is successful the trng does not output real random data, at the end of first part we will reset the device for the second part, in this part we will again generate a trng buffer to see that the same trng output is not generated.

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
    mbed test -t GCC_ARM -m K64F -n tests-trng-basic
    ```

 4. The test should run and display a series of results following the StoragLite API invocations.

## Troubleshooting

If you have problems, you can review the [documentation](https://os.mbed.com/docs/latest/tutorials/debugging.html) for suggestions on what could be wrong and how to fix it.
