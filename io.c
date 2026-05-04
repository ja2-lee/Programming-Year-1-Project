#include "io.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_LINE_MAX 512

static char *trim_in_place(char *text)
{
    while (isspace((unsigned char)*text)) text++;

    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) --end;

    *end = '\0';
    return text;
}

static int parse_double_field(const char *text, double *value_out)
{
    char *end;
    errno = 0;

    double val = strtod(text, &end);
    if (end == text || errno == ERANGE) return -1;

    while (*end && isspace((unsigned char)*end)) end++;
    if (*end != '\0') return -1;

    *value_out = val;
    return 0;
}

static int parse_sample_line(const char *line, WaveformSample *sample)
{
    char buffer[CSV_LINE_MAX];
    snprintf(buffer, sizeof(buffer), "%s", line); // FIXED

    char *fields[8];
    char *token = strtok(buffer, ",");
    size_t i = 0;

    while (token && i < 8) {
        fields[i++] = trim_in_place(token);
        token = strtok(NULL, ",");
    }

    if (i != 8) return -1;

    return parse_double_field(fields[0], &sample->timestamp) ||
           parse_double_field(fields[1], &sample->phase_a_voltage) ||
           parse_double_field(fields[2], &sample->phase_b_voltage) ||
           parse_double_field(fields[3], &sample->phase_c_voltage) ||
           parse_double_field(fields[4], &sample->line_current) ||
           parse_double_field(fields[5], &sample->frequency) ||
           parse_double_field(fields[6], &sample->power_factor) ||
           parse_double_field(fields[7], &sample->thd_percent)
               ? -1 : 0;
}

static size_t count_rows(FILE *fp)
{
    char line[CSV_LINE_MAX];
    size_t count = 0;
    int header = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *t = trim_in_place(line);
        if (!*t) continue;

        if (!header) { header = 1; continue; }
        count++;
    }

    return count;
}

int load_waveform_csv(const char *filename, WaveformSample **samples_out, size_t *count_out)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return -1;
    }

    size_t rows = count_rows(fp);
    fclose(fp);

    if (rows == 0) return -1;

    WaveformSample *samples = malloc(rows * sizeof(*samples));
    if (!samples) return -1;

    fp = fopen(filename, "r");
    if (!fp) {
        free(samples);
        return -1;
    }

    char line[CSV_LINE_MAX];
    size_t index = 0;
    int header_skipped = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *t = trim_in_place(line);
        if (!*t) continue;

        if (!header_skipped) {
            header_skipped = 1;
            continue;
        }

        if (parse_sample_line(t, &samples[index]) != 0) {
            fclose(fp);
            free(samples);
            return -1;
        }

        index++;
    }

    fclose(fp);

    *samples_out = samples;
    *count_out = rows;
    return 0;
}

void free_waveform_samples(WaveformSample *samples)
{
    free(samples);
}

int write_results_report(const char *filename,
                         const WaveformSample *samples,
                         size_t count,
                         const WaveformMetrics *metrics)
{
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror(filename);
        return -1;
    }

    fprintf(fp, "Power Quality Waveform Analysis Report\n");
    fprintf(fp, "=====================================\n\n");

    fprintf(fp, "Samples analysed: %zu\n", count);

    if (samples && count > 0) {
        fprintf(fp, "Time span: %.6f s to %.6f s\n",
                samples[0].timestamp,
                samples[count - 1].timestamp);
    }

    fprintf(fp, "\n");

    for (size_t i = 0; i < PHASE_COUNT; i++) {
        fprintf(fp, "%s\n", phase_name(i));
        fprintf(fp, "  RMS: %.3f V\n", metrics->rms[i]);
        fprintf(fp, "  Peak-to-Peak: %.3f V\n", metrics->peak_to_peak[i]);
        fprintf(fp, "  DC Offset: %.3f V\n", metrics->dc_offset[i]);
        fprintf(fp, "  Std Dev: %.6f\n", metrics->stddev[i]);
        fprintf(fp, "  Clipping Count: %zu\n", metrics->clipping_count[i]);
        fprintf(fp, "  Within Tolerance: %s\n",
                metrics->within_tolerance[i] ? "Yes" : "No");
        fprintf(fp, "  Status Flags: 0x%02X\n\n", metrics->status_flags[i]);
    }

    fclose(fp);
    return 0;
}