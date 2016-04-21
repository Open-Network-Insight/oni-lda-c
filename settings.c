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
    fscanf(fileptr, "var max iter %d\n", &pSettings->VAR_MAX_ITER);
    fscanf(fileptr, "var convergence %f\n", &pSettings->VAR_CONVERGED);
    fscanf(fileptr, "em max iter %d\n", &pSettings->EM_MAX_ITER);
    fscanf(fileptr, "em convergence %f\n", &pSettings->EM_CONVERGED);
    fscanf(fileptr, "alpha %s", alpha_action);
    if (strcmp(alpha_action, "fixed")==0)
    {
        pSettings->ESTIMATE_ALPHA = 0;
    }
    else
    {
        pSettings->ESTIMATE_ALPHA = 1;
    }
    fclose(fileptr);
    return pSettings;
}
