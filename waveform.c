#include "waveform.h"

#include <float.h>
#include <math.h>

#define NOMINAL_RMS 230.0
#define TOLERANCE_FRACTION 0.10
#define CLIPPING_THRESHOLD 324.9

static double get_phase_voltage(const WaveformSample *sample, size_t phase_index)
{
    switch (phase_index) {
    case 0: return sample->phase_a_voltage;
    case 1: return sample->phase_b_voltage;
    case 2: return sample->phase_c_voltage;
    default: return 0.0;
    }
}

const char *phase_name(size_t phase_index)
{
    switch (phase_index) {
    case 0: return "Phase A";
    case 1: return "Phase B";
    case 2: return "Phase C";
    default: return "Unknown Phase";
    }
}

static void compute_phase_metrics(const WaveformSample *samples,
                                  size_t count,
                                  size_t phase_index,
                                  WaveformMetrics *metrics)
{
    double sum = 0.0;
    double sum_of_squares = 0.0;
    double min = DBL_MAX;
    double max = -DBL_MAX;
    size_t clipping = 0;

    const WaveformSample *ptr = samples;

    for (size_t i = 0; i < count; ++i, ++ptr) {
        double v = get_phase_voltage(ptr, phase_index);

        sum += v;
        sum_of_squares += v * v;

        if (v < min) min = v;
        if (v > max) max = v;

        if (fabs(v) >= CLIPPING_THRESHOLD)
            clipping++;
    }

    double mean = sum / count;

    double variance_sum = 0.0;
    ptr = samples;

    for (size_t i = 0; i < count; ++i, ++ptr) {
        double diff = get_phase_voltage(ptr, phase_index) - mean;
        variance_sum += diff * diff;
    }

    double variance = variance_sum / count;
    double rms = sqrt(sum_of_squares / count);
    double stddev = sqrt(variance);

    double lower = NOMINAL_RMS * (1.0 - TOLERANCE_FRACTION);
    double upper = NOMINAL_RMS * (1.0 + TOLERANCE_FRACTION);

    int within = (rms >= lower - 1e-6 && rms <= upper + 1e-6);

    uint8_t flags = 0;
    if (clipping > 0) flags |= STATUS_CLIPPING;
    if (!within) flags |= STATUS_OUT_OF_TOLERANCE;

    metrics->rms[phase_index] = rms;
    metrics->peak_to_peak[phase_index] = max - min;
    metrics->dc_offset[phase_index] = mean;
    metrics->variance[phase_index] = variance;
    metrics->stddev[phase_index] = stddev;
    metrics->clipping_count[phase_index] = clipping;
    metrics->within_tolerance[phase_index] = within;
    metrics->status_flags[phase_index] = flags;
}

void analyse_waveform(const WaveformSample *samples, size_t count, WaveformMetrics *metrics)
{
    for (size_t i = 0; i < PHASE_COUNT; ++i) {
        compute_phase_metrics(samples, count, i, metrics);
    }
}