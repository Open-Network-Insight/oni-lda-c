#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
/*
 * Read settings and return in a newly allocated settings struct.
 *
 */
Settings* read_settings(const char* filename)
{
    Settings* pSettings = (Settings *) malloc(sizeof(Settings));
    FILE* fileptr;
    char alpha_action[100];
    fileptr = fopen(filename, "r");
    fscanf(fileptr, "var max iter %d\n", &pSettings->var_max_iterations);
    fscanf(fileptr, "var convergence %f\n", &pSettings->var_converged);
    fscanf(fileptr, "em max iter %d\n", &pSettings->em_max_iter);
    fscanf(fileptr, "em convergence %f\n", &pSettings->em_converge);
    fscanf(fileptr, "alpha %s", alpha_action);
    if (strcmp(alpha_action, "fixed")==0)
    {
        pSettings->estimate_alpha = 0;
    }
    else
    {
        pSettings->estimate_alpha = 1;
    }
    fclose(fileptr);
    return pSettings;
}
