#include "io.h"
#include "waveform.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.csv>\n", argv[0]);
        return EXIT_FAILURE;
    }

    WaveformSample *samples = NULL;
    size_t count = 0;

    if (load_waveform_csv(argv[1], &samples, &count) != 0) {
        fprintf(stderr, "Failed to load file\n");
        return EXIT_FAILURE;
    }

    if (!samples || count == 0) {
        fprintf(stderr, "No valid data\n");
        return EXIT_FAILURE;
    }

    WaveformMetrics metrics = {0}; // FIXED

    analyse_waveform(samples, count, &metrics);

    if (write_results_report("results.txt", samples, count, &metrics) != 0) {
        fprintf(stderr, "Failed to write report\n");
    }

    free_waveform_samples(samples);

    printf("Done.\n");
    return EXIT_SUCCESS;
}