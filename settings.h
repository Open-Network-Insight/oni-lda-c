#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct
{
    float em_converge;
    int em_max_iter;
    int estimate_alpha;
    double iniitial_alpha;
    int ntopics;
    float var_converged;
    int var_max_iterations;
} Settings;

Settings* read_settings(const char* filename);

#endif

