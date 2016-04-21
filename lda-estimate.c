// (C) Copyright 2004, David M. Blei (blei [at] cs [dot] cmu [dot] edu)

// This file is part of LDA-C.

// LDA-C is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.

// LDA-C is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

#include "lda-estimate.h"
#include "mpi.h"
#include "stdlib.h"
#include "settings.h"
/*
 * perform inference on a document and update sufficient statistics
 *
 */

double doc_e_step(document* doc, double* gamma, double** phi,
                  lda_model* model, lda_suffstats* ss, int VAR_MAX_ITER, float VAR_CONVERGED)
{
    double likelihood;
    int n, k;

    // posterior inference

    likelihood = lda_inference(doc, model, gamma, phi, VAR_MAX_ITER, VAR_CONVERGED);

    // update sufficient statistics

    double gamma_sum = 0;
    for (k = 0; k < model->num_topics; k++)
    {
        gamma_sum += gamma[k];
        ss->alpha_suffstats += digamma(gamma[k]);
    }
    ss->alpha_suffstats -= model->num_topics * digamma(gamma_sum);

    for (n = 0; n < doc->length; n++)
    {
        for (k = 0; k < model->num_topics; k++)
        {
            ss->class_word[k][doc->words[n]] += doc->counts[n]*phi[n][k];
            ss->class_total[k] += doc->counts[n]*phi[n][k];
        }
    }

    ss->num_docs = ss->num_docs + 1;

    return(likelihood);
}


/*
 * writes the word assignments line for a document to a file
 *
 */

void write_word_assignment(FILE* f, document* doc, double** phi, lda_model* model)
{
    int n;

    fprintf(f, "%03d", doc->length);
    for (n = 0; n < doc->length; n++)
    {
        fprintf(f, " %04d:%02d",
                doc->words[n], argmax(phi[n], model->num_topics));
    }
    fprintf(f, "\n");
    fflush(f);
}


/*
 * saves the gamma parameters of the current dataset
 *
 */

void save_gamma(char* filename, double** gamma, int num_docs, int num_topics)
{
    FILE* fileptr;
    int d, k;
    fileptr = fopen(filename, "w");

    for (d = 0; d < num_docs; d++)
    {
	fprintf(fileptr, "%5.10f", gamma[d][0]);
	for (k = 1; k < num_topics; k++)
	{
	    fprintf(fileptr, " %5.10f", gamma[d][k]);
	}
	fprintf(fileptr, "\n");
    }
    fclose(fileptr);
}


/*
 * run_em
 *
 */

void run_em(char* start, char* directory, corpus* corpus, int nproc, const Settings settings)
{
    int d, k, n;
    lda_model *model = NULL;
    double **var_gamma, **var_gamma_local, **phi;
    double wordp[corpus->num_terms], wordp_local[corpus->num_terms];

    int max_iter = settings.var_max_iterations;

    // allocate variational parameters

    var_gamma = malloc(sizeof(double*)*(corpus->num_docs));
    for (d = 0; d < corpus->num_docs; d++)
	var_gamma[d] = malloc(sizeof(double) * settings.ntopics);

    var_gamma_local = malloc(sizeof(double*)*(corpus->num_docs));
    for (d = 0; d < corpus->num_docs; d++)
	var_gamma_local[d] = malloc(sizeof(double) * settings.ntopics);

    int max_length = max_corpus_length(corpus);
    phi = malloc(sizeof(double*)*max_length);
    for (n = 0; n < max_length; n++)
	phi[n] = malloc(sizeof(double) * settings.ntopics);

	//wordp = malloc(sizeof(double)*(5));
	//wordp_local = malloc(sizeof(double)*(5));

    for (n = 0; n < corpus->num_terms; n++)
    {
        wordp[n] = 0.0;
        wordp_local[n] = 0.0;
    }


    // initialize model

    char filename[100];

    lda_suffstats* ss = NULL;
    lda_suffstats* ss_local = NULL;
    if (strcmp(start, "seeded")==0)
    {
        model = new_lda_model(corpus->num_terms, settings.ntopics);
        ss = new_lda_suffstats(model);
        corpus_initialize_ss(ss, model, corpus);
        lda_mle(model, ss, 0);
        model->alpha = settings.iniitial_alpha;
    }
    else if (strcmp(start, "random")==0)
    {
        model = new_lda_model(corpus->num_terms, settings.ntopics);
        ss = new_lda_suffstats(model);
        random_initialize_ss(ss, model);
        lda_mle(model, ss, 0);
        model->alpha = settings.iniitial_alpha;
    }
    else
    {
        model = load_lda_model(start);
        ss = new_lda_suffstats(model);
    }

    sprintf(filename,"%s/000",directory);
    save_lda_model(model, filename);

    // run expectation maximization

    int i = 0;
    double likelihood, likelihood_old = 0, converged = 1;
    double likelihood_local = 0;
     double gamma_local = 0.0; double gamma_global= 0.0;

    sprintf(filename, "%s/likelihood.dat", directory);
    FILE* likelihood_file = fopen(filename, "w");
	int myid, pnum,tag;

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &pnum);

    while (((converged < 0) || (converged > settings.em_converge) || (i <= 2)) && (i <= settings.em_max_iter))
    {
        i++;
        if (myid < 1 ) printf("**** em iteration %d ****\n", i);
        likelihood = 0; likelihood_local = 0;
       // int nstep = 0;

        //int min = 0;
        double alphass = 0.0; double alphass_local = 0.0;
        int numdocs = 0.0; int numdocs_local = 0.0;
        //double wordp = 0.0; double wordp_local = 0.0;
        double wordn = 0.0; double wordn_local = 0.0;

        zero_initialize_ss(ss, model);
        ss_local = ss;

		//printf("**** process %d ****\n", myid);

        for (d = myid; d < corpus->num_docs; d += nproc)
        {
            if ((d % 1000000) == 0) printf("document %d\n",d);
            likelihood_local += doc_e_step(&(corpus->docs[d]),
                                           var_gamma_local[d],
                                           phi,
                                           model,
                                           ss_local,
                                           settings.var_max_iterations,
                                           settings.var_converged);
             //if (d >  corpus->num_docs- 10) printf("**** doc %d  processed****\n",d);
        }
        // may need to do the same thing with sufficient stats
        MPI_Barrier(MPI_COMM_WORLD);
         if (myid < 1) printf("**** reducing suffstats %d ****\n", myid);
        alphass_local = ss_local->alpha_suffstats;
 		MPI_Allreduce(&alphass_local, &alphass, 1, MPI_DOUBLE,
              MPI_SUM, MPI_COMM_WORLD);
        ss->alpha_suffstats = alphass;

        numdocs_local = ss_local->num_docs;
 		MPI_Allreduce(&numdocs_local, &numdocs, 1, MPI_INT,
              MPI_SUM, MPI_COMM_WORLD);
         ss->num_docs = numdocs;
	     // still a problem with word/topic matrix
	     //printf("**** reducing word matrix %d ****\n", myid);
		for (k = 0; k < model->num_topics; k++)
		{
			 if (myid < 1) printf("**** reducing word matrix topic %d , %d ****\n", k,myid);

		 	//for (n = 0; n < corpus->num_terms; n++)
		 	//{
			  //printf("**** reducing word matrix topic %d , %d, %d ****\n", k,n,myid);
			for (n = 0; n < corpus->num_terms; n++)
		 	{
				  wordp_local[n] = ss_local->class_word[k][n] ;
			 }
				  MPI_Allreduce(&wordp_local, &wordp,corpus->num_terms, MPI_DOUBLE,
				  MPI_SUM, MPI_COMM_WORLD);

			for (n = 0; n < corpus->num_terms; n++)
			{
					ss->class_word[k][n] = wordp[n];
			 }
		      //}

			  wordn_local = ss_local->class_total[k];
              MPI_Allreduce(&wordn_local, &wordn, 1, MPI_DOUBLE,
              MPI_SUM, MPI_COMM_WORLD);
              ss->class_total[k] = wordn;

		 }

		MPI_Allreduce(&likelihood_local, &likelihood, 1, MPI_DOUBLE,
              MPI_SUM, MPI_COMM_WORLD);
	    if (myid < 1) printf("**** reducing likelihood %10.10f ****\n", likelihood);
        // m-step

        lda_mle(model, ss, settings.estimate_alpha);

        // check for convergence
        // NOTE: Because likelihood is result of allreduce, all workers agree on the values
        // of likelihood and likelihood_old at all steps... therefore values of max_iter stay in synch

        converged = (likelihood_old - likelihood) / (likelihood_old);
        if (converged < 0) max_iter = max_iter * 2;
        likelihood_old = likelihood;

        // output model and likelihood

        fprintf(likelihood_file, "%10.10f\t%5.5e\n", likelihood, converged);
        fflush(likelihood_file);

        //if ((i % LAG) == 0)
        //{
        //    sprintf(filename,"%s/%03d",directory, i);
        //    save_lda_model(model, filename);
        //    sprintf(filename,"%s/%03d.gamma",directory, i);
            // gather gammas across workers
		//		      	for (k = 0; k < model->num_topics; k++)
		//			  	{
		//			  		if (myid < 1) printf("**** saving gamma matrix topic %d , %d ****\n", k,myid);
		//			  		if (myid > 0)
		//			  		{
		//						for (d = myid; d < corpus->num_docs; d += nproc)
		//			  			{
		//	    					//MPI_Request request;
		//	 						gamma_local = var_gamma_local[d][k];
		//							MPI_Send(&gamma_local,1,MPI_DOUBLE,0,d,MPI_COMM_WORLD);
		//							 if (d >  corpus->num_docs - 5) printf("**** source %d  sent****\n",d);
		//			  			}
		//				    }
		//			  		else
		//			  		{
		//						  //for (d = corpus->num_docs/nproc-1; d < corpus->num_docs; d++) // num_docs -num_docs/nproc
		//						  for (d = corpus->num_docs/nproc + 1; d < corpus->num_docs; d++) // num_docs -num_docs/nproc
		//						  {
		//	    					MPI_Status status;

		//							  MPI_Recv(&gamma_global,1,MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		//						  	  if (status.MPI_TAG > corpus->num_docs - 5) printf("**** source %d  received****\n",status.MPI_TAG);
		//						  	  var_gamma[status.MPI_TAG][k] = gamma_global;
		//						  }
		//						for (d = myid; d < corpus->num_docs; d += nproc)
		//			  			{
			    					//MPI_Request request;
		//	 						gamma_local = var_gamma_local[d][k];
		//							var_gamma[d][k] = gamma_local;
		//			  			}

		//					}
		//					if (myid < 1) printf("*****%d hit barrier*****\n",myid);
		//					MPI_Barrier(MPI_COMM_WORLD);

		//			  	}
					  	// only the first worker on each machine should do this!
		//					if (myid < 1)
		//					{
		//						save_gamma(filename, var_gamma, corpus->num_docs, model->num_topics);
		//					}
    	//				MPI_Barrier(MPI_COMM_WORLD);
		//	}
		MPI_Barrier(MPI_COMM_WORLD);
	}

    // output the final model

    sprintf(filename,"%s/final",directory);
    save_lda_model(model, filename);
    sprintf(filename,"%s/final.gamma",directory);
    // gather gammas across workers
    for (k = 0; k < model->num_topics; k++)
    {
        if (myid == 0) printf("**** saving gamma matrix topic %d  ****\n", k);
        if (myid > 0)
        {
            for (d = myid; d < corpus->num_docs; d += nproc)
            {
                //MPI_Request request;
                gamma_local = var_gamma_local[d][k];
                MPI_Send(&gamma_local,1,MPI_DOUBLE,0, d, MPI_COMM_WORLD);
                if (d >  corpus->num_docs - 5) printf("**** source %d  sent****\n",d);
            }
        }
        else
        {
            for (d=0 ; d < corpus->num_docs; d++) {
                if (d % nproc != 0)  // these documents were handled on other workers and must be gathered
                {
                    MPI_Status status;
                    MPI_Recv(&gamma_global, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG > corpus->num_docs - 5) printf("**** source %d  received****\n", status.MPI_TAG);
                    var_gamma[status.MPI_TAG][k] = gamma_global;
                }
                else
                {
                    gamma_local = var_gamma_local[d][k];
                    var_gamma[d][k] = gamma_local;
                }
            }
        }
        if (myid < 1) printf("*****%d hit barrier*****\n",myid);
        MPI_Barrier(MPI_COMM_WORLD);

	}
	// only the first worker on each machine should do this!
		if (myid < 1)
		{
			printf("*****saving gamma*****\n");
			save_gamma(filename, var_gamma, corpus->num_docs, model->num_topics);
		}
	MPI_Barrier(MPI_COMM_WORLD);

    // output the word assignments (for visualization)
   // if (myid < 1)
   // {
//			sprintf(filename, "%s/word-assignments.dat", directory);
//			FILE* w_asgn_file = fopen(filename, "w");
//			//need to parallel here as well

//			for (d = 0; d < corpus->num_docs; d++)
//			{
//				if ((d % 1000000) == 0) printf("final e step document %d\n",d);
//				likelihood += lda_inference(&(corpus->docs[d]), model, var_gamma[d], phi);
//				write_word_assignment(w_asgn_file, &(corpus->docs[d]), phi, model);
//			}
//			fclose(w_asgn_file);

//	}
	fclose(likelihood_file);
}




/*
 * inference only
 *
 */

void infer(char* model_root, char* save, corpus* corpus, const Settings settings)
{
    FILE* fileptr;
    char filename[100];
    int i, d, n;
    lda_model *model;
    double **var_gamma, likelihood, **phi;
    document* doc;

    model = load_lda_model(model_root);
    var_gamma = malloc(sizeof(double*)*(corpus->num_docs));
    for (i = 0; i < corpus->num_docs; i++)
	var_gamma[i] = malloc(sizeof(double)*model->num_topics);
    sprintf(filename, "%s-lda-lhood.dat", save);
    fileptr = fopen(filename, "w");
    for (d = 0; d < corpus->num_docs; d++)
    {
	if (((d % 100) == 0) && (d>0)) printf("document %d\n",d);

	doc = &(corpus->docs[d]);
	phi = (double**) malloc(sizeof(double*) * doc->length);
	for (n = 0; n < doc->length; n++)
	    phi[n] = (double*) malloc(sizeof(double) * model->num_topics);
	likelihood = lda_inference(doc, model, var_gamma[d], phi, settings.var_max_iterations, settings.var_converged);

	fprintf(fileptr, "%5.5f\n", likelihood);
    }
    fclose(fileptr);
    sprintf(filename, "%s-gamma.dat", save);
    save_gamma(filename, var_gamma, corpus->num_docs, model->num_topics);
}


/*
 * update sufficient statistics
 *
 */



/*
 * main
 *
 */

int main(int argc, char* argv[])
{
    // (est / inf) alpha k settings machinefile data (random / seed/ model) (directory / out)

    const int OPERATION_ARG = 1;


    const char* operation = argv[OPERATION_ARG];

    corpus* corpus;

    long t1;
    (void) time(&t1);
    seedMT(t1);
    // seedMT(4357U);

    Settings* pSettings = (Settings*) malloc(sizeof(Settings));

    if (argc > 1)
    {
        if (strcmp(operation, "est")==0)
        {
            // usage: lda est [alpha] [k] [settings] [#processes] [data] [random/seeded/*] [output directory]

            double alpha = atof(argv[2]); //should read alpha in as a vector instead of from args
            int ntopics = atoi(argv[3]);
            char* settings_path = argv[4];
            int nproc = atoi(argv[5]);
            char* corpus_path = argv[6];
            char* initialization_option = argv[7];
            char* output_directory = argv[8];


            // some settings are read from file, some from the command line

            pSettings  = read_settings(settings_path);
            pSettings->iniitial_alpha = alpha;
            pSettings->ntopics = ntopics;

            corpus = read_data(corpus_path);
            make_directory(output_directory);

            MPI_Init(&argc, &argv);
            run_em(initialization_option, output_directory, corpus, nproc, *pSettings);
            MPI_Finalize();
        }
        if (strcmp(operation, "inf")==0)
        {
            // usage: lda inf [settings] [model] [data] [output filename]

            char* settings_path = argv[2];
            char* model_path = argv[3];
            char* corpus_path = argv[4];
            char* output_name = argv[5];

            read_settings(settings_path);
            corpus = read_data(corpus_path);

            MPI_Init(&argc, &argv);
            infer(model_path, output_name, corpus, *pSettings);
            MPI_Finalize();
        }

        free (pSettings);
    }
    else
    {
        printf("usage : lda est [alpha] [k] [settings] [#processes] [data] [random/seeded/*] [output directory]\n");
        printf("        lda inf [settings] [model] [data] [output filename]\n");
    }
    return(0);
}
