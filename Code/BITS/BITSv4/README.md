# BITS (Balloon Iridium Telemetry System)
The UMDBPP Iridium payload, armed with an Iridium 9603 on a RockBlock carrier board for instantaneous Global Blackmagics

WIP: XBee Mesh net support for other payloads
     New PCB for smaller, lighter, better looking tracking
     Battery Voltage sensing for long haul flights


# Building Code
Hi, from the `BITS` folder, run `arduino-cli compile -e BITSv4` to do reproducible builds. I also created a Github Action that will automatically build the BITSv4 code when you push any changes to the master branch. You can find the build artifacts at [https://github.com/UMDBPP/command_module/releases](https://github.com/UMDBPP/command_module/releases).
