#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

jack_port_t *input_port_left;
jack_port_t *output_port_left;
jack_port_t *input_port_right;
jack_port_t *output_port_right;
jack_client_t *client;

typedef struct cascadestate {
    float s[2];  // filter state
    float c[2];  // filter coefficients
} cascadestate_t;

float cascadefir(float x, cascadestate_t *p) {
    float r = x + (p->s[0] * p->c[0]) +  (p->s[1] * p->c[1]);
    p->s[1] = p->s[0];
    p->s[0] = x;
    return r;
}

void createcascade(float c0,
                   float c1,
                   cascadestate_t *p) {
    p->c[0] = c0;
    p->c[1] = c1;
    p->s[0] = p->s[1] = 0.0f;
}

// left channel

cascadestate_t stage1l;
cascadestate_t stage2l;
cascadestate_t stage3l;
cascadestate_t stage4l;

void initcascadel() {
    createcascade(    0.0f,  1.0f, &stage1l);
    createcascade( 1.4142f,  1.0f, &stage2l);
    createcascade(-1.4142f,  1.0f, &stage3l);
    createcascade(    1.0f,  0.0f, &stage4l);
}

float processCascadel(float x) {
    float v;
    v = cascadefir(x, &stage1l);
    v = cascadefir(v, &stage2l);
    v = cascadefir(v, &stage3l);
    v = cascadefir(v, &stage4l);
    return v*0.125f;
}

// right channel

cascadestate_t stage1r;
cascadestate_t stage2r;
cascadestate_t stage3r;
cascadestate_t stage4r;
cascadestate_t stage5r;
cascadestate_t stage6r;
cascadestate_t stage7r;
cascadestate_t stage8r;
cascadestate_t stage9r;
cascadestate_t stage10r;
cascadestate_t stage11r;
cascadestate_t stage12r;
cascadestate_t stage13r;
cascadestate_t stage14r;
cascadestate_t stage15r;
cascadestate_t stage16r;

void initcascader() {
    createcascade(     1.0f,  0.0f, &stage16r);
    createcascade( 1.96157f,  1.0f, &stage2r);
    createcascade( 1.84776f,  1.0f, &stage4r);
    createcascade( 1.66294f,  1.0f, &stage6r);
    createcascade( 1.41421f,  1.0f, &stage8r);
    createcascade( 1.11114f,  1.0f, &stage10r);
    createcascade( 0.76537f,  1.0f, &stage12r);
    createcascade( 0.39018f,  1.0f, &stage14r);
    createcascade( 0.00000f,  1.0f, &stage1r);
    createcascade(-0.39018f,  1.0f, &stage15r);
    createcascade(-0.76537f,  1.0f, &stage13r);
    createcascade(-1.11114f,  1.0f, &stage11r);
    createcascade(-1.41421f,  1.0f, &stage9r);
    createcascade(-1.66294f,  1.0f, &stage7r);
    createcascade(-1.84776f,  1.0f, &stage5r);
    createcascade(-1.96157f,  1.0f, &stage3r);
}

int stagemonitor = 16;

float processCascader(float x) {
    float v;

    v = cascadefir(x, &stage1r);
    v = cascadefir(v, &stage2r);
    v = cascadefir(v, &stage3r);
    v = cascadefir(v, &stage4r);
    v = cascadefir(v, &stage5r);
    v = cascadefir(v, &stage6r);
    v = cascadefir(v, &stage7r);
    v = cascadefir(v, &stage8r);
    v = cascadefir(v, &stage9r);
    v = cascadefir(v, &stage10r);
    v = cascadefir(v, &stage11r);
    v = cascadefir(v, &stage12r);
    v = cascadefir(v, &stage13r);
    v = cascadefir(v, &stage14r);
    v = cascadefir(v, &stage15r);
    v = cascadefir(v, &stage16r);
    return v*0.03125f;
}

int process (jack_nframes_t nframes, void *arg) {
  jack_default_audio_sample_t *in, *out;

  in = jack_port_get_buffer (input_port_left, nframes);
  out = jack_port_get_buffer (output_port_left, nframes);
  for (int i=0; i<nframes; i++)
    out[i] = processCascadel(in[i]);

  in = jack_port_get_buffer (input_port_right, nframes);
  out = jack_port_get_buffer (output_port_right, nframes);
  for (int i=0; i<nframes; i++)
    out[i] = processCascader(in[i]);

  return 0;
}

void jack_shutdown (void *arg) {
        exit (1);
}

int main (int argc, char *argv[]) {
  const char **ports;
  const char *client_name = "simple";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback (client, process, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  initcascadel();
  initcascader();

  printf ("engine sample rate: %" PRIu32 "\n", jack_get_sample_rate (client));

  input_port_left = jack_port_register (client, "input_left",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_left = jack_port_register (client, "output_left",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
  input_port_right = jack_port_register (client, "input_right",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_right = jack_port_register (client, "output_right",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

  if ((input_port_left == NULL) || (output_port_left == NULL) ||
      (input_port_right == NULL) || (output_port_right == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }

  while (1)
    sleep(10);

  jack_client_close (client);

  exit (0);
}
