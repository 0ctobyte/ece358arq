#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "sim.h"
#include "rv.h"

static char *simulator_types[][2] = {
  {"ABP", "Alternating Bit Protocol Simulator"},
  {"ABP_NAK", "Alternating Bit Protocol With NAK Simulator"},
  {"GBN", "Go Back N Simulator"}
};

void parse_cmdline_args(int32_t argc, char **argv, sim_inputs_t *args) {
  char *optstring = "P:N:c:l:h:t:b:d:n";
  
  if(argc == 1) {
    fprintf(stderr, "Usage: %s [-P npackets] [-N wsize] [-c bps] [-l plength] [-h hlength] [-t tau] [-b ber] [-d timeout_multiplier] [-n]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  for(int32_t opt = getopt(argc, argv, optstring); opt != -1; opt = getopt(argc, argv, optstring)) {
    switch(opt) {
      case 'P':
        {
          uint64_t P = strtoull(optarg, NULL, 0);
          if(P != 0) args->P = P;
          break;
        }
      case 'N':
        {
          uint64_t N = strtoull(optarg, NULL, 0);
          if(N != 0) args->N = N;
          break;
        }
      case 'c':
        {
          uint64_t c = strtoull(optarg, NULL, 0);
          if(c != 0) args->C = c;
          break;
        }
      case 'l':
        {
          uint64_t l = strtoull(optarg, NULL, 0);
          if(l != 0) args->l = l;
          break;
        }
      case 'h':
        {
          uint64_t h = strtoull(optarg, NULL, 0);
          if(h != 0) args->H = h;
          break;
        }
      case 't':
        {
          double t = strtod(optarg, NULL);
          if(!(t < 1e-15)) args->tau = t;
          break;
        }
      case 'b':
        {
          double b = strtod(optarg, NULL);
          if(!(b < 1e-15)) args->ber = b;
          break;
        }
      case 'd':
        {
          double d = strtod(optarg, NULL);
          if(!(d < 1e-15)) args->td = d*args->tau;
          break;
        }
      case 'n':
        {
          args->nak = true;
          break;
        }
      default:
        fprintf(stderr, "Usage: %s [-P npackets] [-N wsize] [-c bps] [-l plength] [-h hlength] [-t tau] [-b ber] [-d timeout_multiplier] [-n]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
}

int32_t main(int32_t argc, char **argv) {
    rv_seed();

    sim_inputs_t inputs = INPUTS_DEFAULT;

    parse_cmdline_args(argc, argv, &inputs);

    uint64_t sim_type = (inputs.N == 1) ? ((inputs.nak) ? 1 : 0) : 2;

    sim_state_t state = STATE_DEFAULT;
    sim_outputs_t outputs = OUTPUTS_DEFAULT;

    printf("====================================================================================================\n");

    printf("\nSIMULATOR\n{\n\ttype:\t%-25s (%s)\n}\n\n", simulator_types[sim_type][0], simulator_types[sim_type][1]); 
    printf("\nINPUTS\n{\n\tP:\t%-25llu (Simulation period [successful packets])"
        "\n\ttd:\t%-25f (Timeout period)"
        "\n\tN:\t%-25llu (Window size [packets])"
        "\n\tber:\t%-25f (Bit Error Rate)"
        "\n\tC:\t%-25llu (Transmisson rate of output link [bits/sec])"
        "\n\tH:\t%-25llu (Frame header size [bytes])"
        "\n\tl:\t%-25llu (Packet length [bytes])"
        "\n\ttau:\t%-25f (Propagation delay [s])"
        "\n\tnak:\t%-25s (Is NAK enabled?)\n}\n\n", 
        inputs.P,inputs.td,inputs.N,inputs.ber,inputs.C,inputs.H,inputs.l,inputs.tau, inputs.nak ? "yes" : "no");
  
    sim_gen_timeout(&state, &inputs);
    sim_send(&state, &inputs, &outputs);
    for(es_event_t event; es_pq_size(state.es)>0;) {
      event = es_pq_dequeue(state.es);
      //printf("type=%c, time=%f, rn=%llu, has_error=%d\n", event.event_type==0?'T':'A', event.time, event.rn, event.corrupted);
      state.event = event;
      switch(event.event_type) {
        case TIMEOUT:
          {
            sim_event_timeout(&state, &inputs, &outputs);
            break;
          }
        case ACK:
          {
            sim_event_ack(&state, &inputs, &outputs);
            break;
          }
      }
    } 

    printf("\nSTATE\n{\n\tsn:\t%-25llu (Sequence number of sent packet)"
        "\n\tnack:\t%-25llu (Next expected ACK #)"
        "\n\tnsn:\t%-25llu (Next expected sequence #)"
        "\n\tNp:\t%-25llu (Total # of packets sent)"
        "\n\tNs:\t%-25llu (# of packets sent successfully)"
        "\n\tNt:\t%-25llu (# of timeout events)\n}\n\n", 
        state.sn, state.nack, state.nsn, state.Np, state.Ns, state.Nt);

    printf("\nOUTPUTS\n{\n\ttput:\t%-25f (Throughput, excluding header bits [bps])\n}\n\n", outputs.tput);

    printf("====================================================================================================\n");

    es_pq_delete(state.es);

		return 0;
}

