#ifndef LDA_ESTIMATE_H
#define LDA_ESTIMATE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>

#include "lda.h"
#include "lda-data.h"
#include "lda-inference.h"
#include "lda-model.h"
#include "lda-alpha.h"
#include "utils.h"
#include "cokus.h"
#include "settings.h"

double doc_e_step(document* doc,
                  double* gamma,
                  double** phi,
                  lda_model* model,
                  lda_suffstats* ss,
                  int VAR_MAX_ITER,
                  float VAR_CONVERGED);

void save_gamma(char* filename,
                double** gamma,
                int num_docs,
                int num_topics);

void run_em(char* start,
            char* directory,
            corpus* corpus,
            int const nproc,
            const Settings settings
);


void infer(char* model_root,
           char* save,
           corpus* corpus,
           const Settings settings);

#endif


