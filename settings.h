#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct
{
    float EM_CONVERGED;
    int EM_MAX_ITER;
    int ESTIMATE_ALPHA;
    double INITIAL_ALPHA;
    int NTOPICS;
    float VAR_CONVERGED;
    int VAR_MAX_ITER;
} Settings;

Settings* read_settings(const char* filename);

#endif

