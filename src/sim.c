#include <assert.h>
#include <stdio.h>

#include "sim.h"
#include "rv.h"

typedef struct {
  bool lost;
  bool corrupted;
  uint64_t sn;
  double time;
} sim_channel_t;

sim_channel_t _sim_channel(double BER, double tau, double tc, uint64_t sn, uint64_t L) {
  // Generate bit error probabilities for each of the L bits
  uint64_t biterr_count = 0;
  for(uint64_t i = 0; i < L; ++i) biterr_count += (rv_uniform() < BER) ? 1 : 0; 
  sim_channel_t e = {.lost=(biterr_count > 4) ? true : false, .time=tc+tau, .sn=sn, .corrupted=(biterr_count > 0) ? true : false};
  return e;
}

void sim_gen_ack(sim_state_t *state, double time, uint64_t rn, bool corrupted) {
  es_event_t ack = {.event_type = ACK, .time = time, .rn = rn, .corrupted = corrupted};
  es_pq_enqueue(state->es, ack);
}

void sim_gen_timeout(sim_state_t *state, sim_inputs_t *inputs) {
  double transmission_delay = ((double)(inputs->H+inputs->l)*8.0)/inputs->C;

  // Need to rewind time to find the new timeout value
  state->td = state->ti - (transmission_delay * ((inputs->N - state->P + state->sn + 1) % (inputs->N+1))) + transmission_delay + inputs->td;
  es_event_t timeout = {.event_type = TIMEOUT, .time = state->td, .rn = state->sn, .corrupted = false};
  es_pq_enqueue(state->es, timeout);
}

void sim_event_timeout(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // If the # of successfully sent packets is enough, then don't transmit anymore frames
  if(state->Ns == inputs->S) return;

  // Check if the timeout event is invalid
  if(state->event.time < state->td || state->event.time > state->td) return;

  // Increment the timeout counter
  ++state->Nt;

  // Schedule all frames in the buffer to be Retransmitted
  state->sn = state->P;
  sim_gen_timeout(state, inputs);
  sim_send(state, inputs, outputs);
}

void sim_event_ack(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  if(state->Ns == inputs->S) return;

  // Check if RN is a valid ack sequence number, that must mean it is within the valid range of P+1 to SN mod N+1
  bool valid_rn = false;
  for(uint64_t i = ((state->P+1) % (inputs->N+1)); i != ((state->sn+1) % (inputs->N+1)); i = ((i+1) % (inputs->N+1))) {
    if(state->event.rn == i) {
      valid_rn = true;
      break;
    }
  }

  // If the sender has received an ack without error, increment P and next expected ack
  if(valid_rn && !state->event.corrupted) {
    state->Ns += ((inputs->N - state->P + state->event.rn + 1) % (inputs->N+1));
    state->P = (inputs->N + state->event.rn + 1) % (inputs->N+1);
    state->nack = (state->P+1) % (inputs->N+1);

    // If the # of successfully sent packets is enough, then don't transmit anymore frames
    if(state->Ns == inputs->S) {
      outputs->tput = (double)(inputs->l*8*state->Ns)/state->ti;
      return;
    }

    // Transmit the next frame
    sim_gen_timeout(state, inputs);
    sim_send(state, inputs, outputs);
  } else if(inputs->N == 1 && inputs->nak) {
    // If the ACK frame is corrupted but NAK retransmission is enabled, resend the frame
    state->sn = state->P;
    sim_gen_timeout(state, inputs);
    sim_send(state, inputs, outputs);
  }
}

void sim_send(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // Don't send anything if N packets have been sent, wait for a timeout or an ack. Ignore this if enough successfully packets have been sent
  if(((inputs->N - state->P + state->sn + 1) % (inputs->N+1)) == inputs->N || state->Ns == inputs->S) {
    state->ti = es_pq_at(state->es, 1).time;
    return;
  }

  // First the sent frame must be passed into the channel
  double transmission_delay = ((double)(inputs->H+inputs->l)*8.0)/inputs->C;

  // The time it takes to send the packet completely into the channel 
  state->ti += transmission_delay;

  sim_channel_t e = _sim_channel(inputs->ber, inputs->tau, state->ti, state->sn, (inputs->H+inputs->l)*8);

  // Increment the sequence number
  state->sn = (state->sn+1) % (inputs->N+1);
  ++state->Np;

  // If the frame is lost, nothing to do
  if(e.lost) return;

  /* BEGIN RECIEVER */

  // The receiver receives the frame at time tcs
  state->tcs = e.time;

  // If the receiver receives a frame without error, increment the next expected frame
  if(!e.corrupted && e.sn == state->nsn) {
    state->nsn = (state->nsn+1) % (inputs->N+1);
  }

  /* END RECIEVER */

  // The ACK frame must be passed through the channel
  transmission_delay = (double)(inputs->H*8)/inputs->C;

  // The time it takes to send the ACK packet to the channel
  state->tcs += transmission_delay;

  e = _sim_channel(inputs->ber, inputs->tau, state->tcs, state->nsn, inputs->H*8);

  // If the ack is lost, don't create an event
  if(e.lost) return;

  // Generate the ACK event (with or without errors)
  sim_gen_ack(state, e.time, e.sn, e.corrupted);
}

