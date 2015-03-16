#include "sim.h"

typedef enum {
  NOERROR,
  ERROR,
  LOST
} error_t;

void sim_gen_ack(sim_state_t *state, sim_inputs_t *inputs) {
  // First get the probability that the sent frame is received correctly
  error_t e = NOERROR;
  // has_error = channel(state->tc, state->sn, inputs->H+inputs->l);
  
  // The receiver receives the frame at time tcs
  state->tcs = state->tc + inputs->tau;

  // If the receiver receives a frame without error, increment the next expected frame
  if(e == NOERROR && state->sn == state->nsn) {
    state->nsn = (state->nsn+1) % (inputs->N+1);
  }

  // Get the probability that the ack frame is sent correctly
  e = NOERROR;
  // e = channel(state->tcs, state->nsn, inputs->H);

  // The sender receives the ack at time tc
  state->tc = state->tcs + inputs->tau;

  // If the ack is lost, don't create an event
  if(e == LOST) return;

  es_event_t ack = {.event_type = ACK, .time = state->tc, .rn = state->nsn, .has_errors = (e == NOERROR) ? false : true};
  es_pq_enqueue(state->es, ack);
}

void sim_gen_timeout(sim_state_t *state, sim_inputs_t *inputs) {
  double transmission_delay = ((double)(inputs->H+inputs->l))/inputs->C;
  state->td = state->tc + transmission_delay + inputs->td;
  es_event_t timeout = {.event_type = TIMEOUT, .time = state->td, .rn = 0, .has_errors = false};
  es_pq_enqueue(state->es, timeout);
}

void sim_event_timeout(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // Check if the timeout event is invalid
  if(state->event.time < state->td) return;

  ++outputs->Nt;

  // Generate new timeout event and simulate the sending of the new 'frame' with sim_gen_ack
  sim_gen_timeout(state, inputs);
  sim_gen_ack(state, inputs);
}

void sim_event_ack(sim_state_t *state, sim_inputs_t *inputs, sim_outputs_t *outputs) {
  // If the sender has received an ack without error, increment sequence number and next expected ack
  if(state->event.has_errors == false && state->event.rn == state->nack) {
    state->sn = (state->sn+1) % (inputs->N+1);
    state->nack = (state->nack+1) % (inputs->N+1);
    ++outputs->Ns;
  }
  
  ++outputs->Np;
  // send next frame (generate timeout for new frame)
  sim_gen_timeout(state, inputs);
  sim_gen_ack(state, inputs);
}

