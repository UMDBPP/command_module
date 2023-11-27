# ```</CMD>```

Command is the primary tracking payload flown by BPP. This repository contains design files and software required to run the current version of command.

Current POC:
Shipman + JB

 ## Design Priciples
As the command module needs to fly on every flight mass reduction is a primary focus. For this reason a switch from perfboard to custom PCB's was done, and a substantial reduction in box volume is underway. Reliability is incredibly important and for this reason the payload operates its tracking systems in a full 4N redundant method.
### Tracking systems include (system/function):
#### 2x LiteAPRS 2m transmitters on 144.390 MHz
In flight position updates every minute. By nature of APRS tracking in hilly terrain unlikely to receive exact coordinates under 500m unless operating our own radios.
#### 1x SPOT Trace Tracker
COTS tracker from SPOT. Gives exact coordinates on landing.
#### 1x BITS (Balloon Iridium Telemetry System)
Exact coordinates throughout the flight, variable transmission rate 1min-15minutes (technically 10s-inf). Datalogger. XBee 900 HP allows for commands to be relayed through Iridium to other payloads and telemetry from payloads to be sent to ground. Possibility of ground to payload XBee communication without Iridium allowing for low latency high rate commanding.
