#ifndef __SIM_H__
#define __SIM_H__

#include <stdint.h>
#include "es.h"

#define INPUTS_DEFAULT {.P=10000,.td=2.5*0.005,.N=1,.ber=0.0,.tau=0.005,.H=54,.l=1500,.C=5000000,.nak=false};
#define STATE_DEFAULT {.sn=0,.nack=1,.nsn=0,.tc=0.0,.tcs=0.0,.td=0.0,.Np=0,.Ns=0,.Nt=0,.event={0,0.0,0,false},es_pq_create(10)}
#define OUTPUTS_DEFAULT {0}

// Inputs into the simulator
typedef struct {
  // Period of simulation, i.e. Simulate till P, the # of successfully delivered packets
  uint64_t P;
  
  // The timeout delta in milliseconds
  double td;

  // The window size
  uint64_t N;

  // Bit error rate
  double ber;

  // Fixed length frame header size in bytes
  uint64_t H;

  // Length of Layer 3 packet in bytes
  uint64_t l;

  // Channel propagation delay
  double tau;

  // Transmission rate of output link in bits per second
  uint64_t C;
  
  // NAK enabled?
  bool nak;
} sim_inputs_t;

// Holds the state of the simulator
typedef struct {
  // The sequence number of the current packet being sent by the sender
  uint64_t sn;

  // The sender expects this sequence number from the next ACK packet
  uint64_t nack;

  // The receiver expects the next received frame to have this sequence number 
  uint64_t nsn;

  // The sender's current simulation time
  double tc;

  // The receiver's current simulation time
  double tcs;

  // New timeout value. This is used to invalidate old timeout events in the ES
  double td;

  // Total # of packets sent
  uint64_t Np;
  
  // Number of successfully sent packets
  uint64_t Ns;

  // Number of timeouts
  uint64_t Nt;

  // Current event being processed
  es_event_t event;

  // Discrete event scheduler
  es_pq_t *es;
} sim_state_t;

typedef struct {
  // The throughput in bits/sec
  double tput;
} sim_outputs_t;

// Generate ACK event in the ES with the given parameters
void sim_gen_ack(sim_state_t *state, double time, uint64_t rn, bool has_errors);

// Generate TIMEOUT event in the ES
void sim_gen_timeout(sim_state_t *state, sim_inputs_t *inputs);

// Handle TIMEOUT event
void sim_event_timeout(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs);

// Handle ACK event
void sim_event_ack(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs);

// Simulate packet sending
void sim_send(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs);

#endif // __SIM_H__
