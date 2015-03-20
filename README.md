# Data Link Layer & ARQ Protocols
An ARQ protocol simulator for the ECE 358 computer networks course.

## Build
Just run `make` in the top-level directory
And `make clean` to remove compiled object files and other generated data files 

## Usage
Usage: `./sim [-S npackets] [-N wsize] [-c bps] [-l plength] [-h hlength] [-t tau] [-b ber] [-d tratio] [-n]`

* `-S` (Number of packets that need to be sent successfully for the simulator to terminate)
* `-N` (The window size in packets. If this is set to 1 then ABP simulation is selected, otherwise GBN simulation will be performed)
* `-c` (The simulated link rate of the channel in bps)
* `-l` (The length of a packet in bytes)
* `-h` (The length of the frame header in bytes)
* `-t` (Tau, the propagation delay of the channel in seconds)
* `-b` (The bit error rate (must be between 0 and 1). Set to 0 for no bit errors)
* `-d` (The timeout period is set to this value (i.e. timeout/tau) times tau)
* `-n` (Specify NAK retransmission. If N=1 and this option is given, then the simulator used will be ABP_NAK)

#### Scripts
* `scripts/run_ABP` to run the ABP simulator and produce the required CSV file
* `scripts/run_ABP_NAK` to run the ABP_NAK simulator and produce the required CSV file
* `scripts/run_GBN` to run the GBN simulator and produce the required CSV file
* `scripts/plot` to generate png images of all the required plots. The above scripts must be run before this.

## Code
* **src/es.c** - Array (heap) based priority queue implementation
* **src/rv.c** - Uniform random variable generators
* **src/sim.c** - Simulator event procedures and event generation functions
* **src/main.c** - Program entry and setup

