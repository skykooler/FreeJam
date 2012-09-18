#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sndfile.h>
#include <portaudio.h>

#define WARN(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#define DIE(...) do { WARN(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define PA_ENSURE(...)                                  \
    do {                                        \
        if ((err = __VA_ARGS__) != paNoError) {                 \
            WARN("PortAudio error %d: %s\n", err, Pa_GetErrorText(err));    \
            goto error;                         \
        }                                   \
    } while (0)


int main(int argc, char **argv)
{
    SF_INFO sfi;
    SNDFILE *sf;

    sf_count_t nread;
    const size_t nframes = 512;
    float *buf;

    PaStream *stream;
    int err;

    /* Open the input file */
    if (argc < 2)
        DIE("Syntax: %s <filename>\n", argv[0]);
    memset(&sfi, 0, sizeof(sfi));
    if ((sf = sf_open(argv[1], SFM_READ, &sfi)) == NULL)
        DIE("Could not open \"%s\".\n", argv[1]);

    /* Allocate buffer */
    if ((buf = malloc(nframes * sfi.channels * sizeof(float))) == NULL)
        DIE("Could not malloc.");

    /* Initialise PortAudio and open stream for default output device */
    PA_ENSURE( Pa_Initialize() );
    /* Assume that the audio device can handle the input file's sample rate
       and number of channels */
    PA_ENSURE( Pa_OpenDefaultStream(&stream, 0, sfi.channels, paFloat32, sfi.samplerate,
                    paFramesPerBufferUnspecified, NULL, NULL) );
    PA_ENSURE( Pa_StartStream(stream) );

    /* Write file data to stream */
    while ((nread = sf_readf_float(sf, buf, nframes)) > 0)
        PA_ENSURE( Pa_WriteStream(stream, buf, nread) );

    /* Clean up */
    PA_ENSURE( Pa_StopStream(stream) );
    PA_ENSURE( Pa_CloseStream(stream) );
error:
    Pa_Terminate();
    sf_close(sf);
    free(buf);

    return err != paNoError;
}
