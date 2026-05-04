#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <stddef.h>
#include <stdint.h>

#define PHASE_COUNT 3

#define STATUS_CLIPPING 0x01
#define STATUS_OUT_OF_TOLERANCE 0x02

typedef struct {
    double timestamp;
    double phase_a_voltage;
    double phase_b_voltage;
    double phase_c_voltage;
    double line_current;
    double frequency;
    double power_factor;
    double thd_percent;
} WaveformSample;

typedef struct {
    double rms[PHASE_COUNT];
    double peak_to_peak[PHASE_COUNT];
    double dc_offset[PHASE_COUNT];
    double variance[PHASE_COUNT];
    double stddev[PHASE_COUNT];
    size_t clipping_count[PHASE_COUNT];
    int within_tolerance[PHASE_COUNT];
    uint8_t status_flags[PHASE_COUNT];
} WaveformMetrics;

const char *phase_name(size_t phase_index);
void analyse_waveform(const WaveformSample *samples, size_t count, WaveformMetrics *metrics);

#endif