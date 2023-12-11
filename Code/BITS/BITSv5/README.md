# ```</BITSv5 Software>```

This folder contains the code needed to operate BITS (Balloon Iridium Telemetry System) Version 5. The below outlines the structure of the code. BITS Version 5 uses the RP2040 by the Raspberry Pi foundation and leverages the RP2040 Pico C SDK and build system meaning that this is not an Arduino based project!

The test project below currently features 3 executable binaries that are automatically built using a GitHub action. This is a proof of concept and enables the BITSv5 software to be built independent of any single user which makes the process of changing and rebuilding the project more approachable to everyone.

The action that builds the test project can be seen here: [https://github.com/UMDBPP/command_module/actions/workflows/build-BITSv5.yml](https://github.com/UMDBPP/command_module/actions/workflows/build-BITSv5.yml)

And the release product that it creates can be seen here: [https://github.com/UMDBPP/command_module/releases/tag/BITSv5-v0.0.0](https://github.com/UMDBPP/command_module/releases/tag/BITSv5-v0.0.0)

## Test
This is the current working space for developing BITSv5 software. Currently basic radio communications have been implemented and verified with hardware. Below are features that need to be implemented for a minimum viable product in the the next month or two.

[x] Basic long range radio communications

[] Serial console

[] Non-volatile configuration

[] GPS geolocation

[] Iridium interface and comms

## Radio Test
This is a future project designed to be a suite of tests that test the radio found on BITSv5. Not currently implemented.

## Pico-SDK
This is a submodule that pulls in the RP2040 SDK which is required to build all of the code found in this folder. It shouldn't need to be touched except for the occasional update.

More on the RP2040 C SDK can be found here: [https://www.raspberrypi.com/documentation/pico-sdk/](https://www.raspberrypi.com/documentation/pico-sdk/)

## Reference
This folder contains two useful documents from the Raspberry Pi Foundation about using the SDK above.