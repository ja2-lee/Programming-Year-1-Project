#ifndef IO_H
#define IO_H

#include <stddef.h>

#include "waveform.h"

int load_waveform_csv(const char *filename, WaveformSample **samples_out, size_t *count_out);
int write_results_report(const char *filename,
                         const WaveformSample *samples,
                         size_t count,
                         const WaveformMetrics *metrics);
void free_waveform_samples(WaveformSample *samples);

#endif