#include "sim.h"

typedef struct {
  bool lost;
  bool corrupted;
  uint64_t sn;
  double time;
} sim_channel_t;

sim_channel_t _sim_channel(double tau, double tc, uint64_t sn, uint64_t flen) {
  sim_channel_t e = {.lost=false, .time=tc+tau, .sn=sn, .corrupted=false};
  return e;
}

void sim_gen_ack(sim_state_t *state, double time, uint64_t rn, bool corrupted) {
  es_event_t ack = {.event_type = ACK, .time = time, .rn = rn, .corrupted = corrupted};
  es_pq_enqueue(state->es, ack);
}

void sim_gen_timeout(sim_state_t *state, sim_inputs_t *inputs) {
  double transmission_delay = ((double)(inputs->H+inputs->l))/inputs->C;
  state->td = state->tc + transmission_delay + inputs->td;
  es_event_t timeout = {.event_type = TIMEOUT, .time = state->td, .rn = state->sn, .corrupted = false};
  es_pq_enqueue(state->es, timeout);
}

void sim_event_timeout(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // Check if the timeout event is invalid
  if(state->event.time < state->td) return;

  // If the # of successfully sent packets is enough, then don't transmit anymore frames
  if(state->Ns == inputs->P) return;

  // Update the sender's time
  state->tc = state->event.time;
  ++state->Nt;

  // Retransmit the frame
  sim_gen_timeout(state, inputs);
  sim_send(state, inputs, outputs);
}

void sim_event_ack(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // Update the sender's time
  state->tc = state->event.time;

  // If the sender has received an ack without error, increment sequence number and next expected ack
  if(!state->event.corrupted && state->event.rn == state->nack) {
    state->sn = (state->sn+1) % (inputs->N+1);
    state->nack = (state->nack+1) % (inputs->N+1);
    ++state->Ns;

    // If the # of successfully sent packets is enough, then don't transmit anymore frames
    if(state->Ns == inputs->P) return;

    // Transmit the next frame
    sim_gen_timeout(state, inputs);
    sim_send(state, inputs, outputs);
  } else if(state->event.corrupted && inputs->nak) {
    // If the ACK frame is corrupted but NAK retransmission is enabled, resend the frame
    sim_gen_timeout(state, inputs);
    sim_send(state, inputs, outputs);
  }
}

void sim_send(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  ++state->Np;

  // First the sent frame must be passed through the channel
  double transmission_delay = ((double)(inputs->H+inputs->l))/inputs->C;
  sim_channel_t e = _sim_channel(inputs->tau, state->tc+transmission_delay, state->sn, inputs->H+inputs->l);
 
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
  transmission_delay = (double)inputs->H/inputs->C;
  e = _sim_channel(inputs->tau, state->tcs+transmission_delay, state->nsn, inputs->H);

  // If the ack is lost, don't create an event
  if(e.lost) return;

  // Generate the ACK event (with or without errors)
  sim_gen_ack(state, e.time, e.sn, e.corrupted);
}

