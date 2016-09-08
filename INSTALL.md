# oni-lda-c

Parallel implementation of latent Dirchlet allocation for Open-Network-Insight, version 1.0.1

oni-lda-c is a program for *topic modelling*:  From a collection of documents and integer k, infer k topics and assign to each document a mixture of topics and to each topic a mixture of words. 

Topic modelling is used in the Open-Network-Insight suspicious connections analysis to generate profiles of typical traffic on the network and estimate
the probability of a given communication occurring between two IPs.  Network traffic on a particular channel corresponds to a "document" and
summaries of the traffic are the "words". 

For example, in a netflow analysis the document is communication between two IP addresses and
and the "words" are summaries of the netflow records between the two. The
"topics" are then profiles of common traffic on the network, the mixture of topic to the documents
is a decomposition of the traffic between two IPs into these common traffic profiles.  For given communication, we can estimate its probablity of occurrence between a given IP pair and flag it as "suspicious".  Similarly, in an analysis based on the DNS protocols, the documents are the DNS queries
coming from a given client IP address, and the words are summaries of the DNS responses.


To learn more about topic modelling, we suggest that you start with [these materials](https://www.cs.princeton.edu/~blei/topicmodeling.html).

**On the shoulders of giants**: oni-lda-c is an MPI parallelization of [the BleiLab's single processor lda-c program](https://github.com/blei-lab/lda-c).

## Getting Started

These instructions are for installing oni-lda-c as a standalone topic modelling tool. If you wish to install oni-lda-c as part of Open-Network-Insight, please follow the instructions [here for version 1.0.1](https://github.com/Open-Network-Insight/open-network-insight/wiki). 
Even if you wish only to use oni-lda-c as a part of oni, the material in this README file may be helpful for configuring and understanding oni-lda-c.

### Prerequisites

oni-lda-c requires an MPI installation on the cluster. It has been validated with [mpich](http://www.mpich.org/) and [intel-mpi](https://software.intel.com/en-us/intel-mpi-library). You must install your favorite MPI on your cluster following the instructions provided by that
distribution.

### Compilation

Compilation requires that MPI libraries have been installed on the machine where oni-lda-c will be compiled. Other than that, just make in the top oni-lda-c directory:
```
[user@system  ~]$ cd oni-lda-c/
[user@system  oni-lda-c]$ make clean ; make
rm -f *.o
mpicc -O3 -Wall -g   -c -o lda-data.o lda-data.c
mpicc -O3 -Wall -g   -c -o lda-estimate.o lda-estimate.c
lda-estimate.c: In function ‘run_em’:
lda-estimate.c:181: warning: unused variable ‘tag’
mpicc -O3 -Wall -g   -c -o lda-model.o lda-model.c
mpicc -O3 -Wall -g   -c -o lda-inference.o lda-inference.c
mpicc -O3 -Wall -g   -c -o utils.o utils.c
mpicc -O3 -Wall -g   -c -o cokus.o cokus.c
mpicc -O3 -Wall -g   -c -o lda-alpha.o lda-alpha.c
mpicc -O3 -Wall -g lda-data.o lda-estimate.o lda-model.o lda-inference.o utils.o cokus.o lda-alpha.o -o lda -lm
[user@system oni-lda-c]$ ls lda
lda
```
###  Configuration

There are two files that must be configured to run oni-lda-c: a *machine file* that tells MPI what the hosts are and how many processes to run on each host,
and a *settings* file that contains settings that control the behavior of LDA.

**Configure the machine file** 
There is a sample machinefile in `oni-lda-c/machinefile`. Edit your machine file as follows:
```
[user@system ~]]$ cat oni-lda-c/machinefile 
Host_1:p_1
Host_2:p_2
Host_3:p_3
...
Host_N:p_N 
```

where ```Host_i``` is the name of  the i-th MPI host and `p_i` is the number of processes to run on the i-th host.  

For example, consider four worker nodes running 5 processes each:
```
[user@system ~]$ cat oni-lda-c/machinefile 
mynode1:5
mynode2:5
mynode3:5
mynode4:5 
```

**Configure LDA settings** 

See `oni-lda-c/settings.txt` for an example.

This is of the following form:

     var max iter [integer e.g., 10 or -1]
     var convergence [float e.g., 1e-8]
     em max iter [integer e.g., 100]
     em convergence [float e.g., 1e-5]
     alpha [fit/estimate]

where the settings are

     [var max iter]

     The maximum number of iterations of coordinate ascent variational
     inference for a single document.  A value of -1 indicates "full"
     variational inference, until the variational convergence
     criterion is met.

     [var convergence]

     The convergence criteria for variational inference.  Stop if
     (score_old - score) / abs(score_old) is less than this value (or
     after the maximum number of iterations).  Note that the score is
     the lower bound on the likelihood for a particular document.

     [em max iter]

     The maximum number of iterations of variational EM.

     [em convergence]

     The convergence criteria for varitional EM.  Stop if (score_old -
     score) / abs(score_old) is less than this value (or after the
     maximum number of iterations).  Note that "score" is the lower
     bound on the likelihood for the whole corpus.

     [alpha]

     If set to [fixed] then alpha does not change from iteration to
     iteration.  If set to [estimate], then alpha is estimated along
     with the topic distributions.
     
### Prepare data for input 

The corpus of documents and words must be preprocessed into a single file for ingestion by oni-lda-c
In this representation, each document is represented as a sparse vector of word
counts. That is, the data is a text file where each line is of the form:

     [M] [term_1]:[count] [term_2]:[count] ...  [term_N]:[count]

where [M] is the number of unique terms in the document, and the
[count] associated with each term is how many times that term appeared
in the document.  Note that [term_1] is an integer which indexes the
term; it is not a string.

This file must be loaded onto each MPI host in the working directories which MPI will use (typically, by default this
is a directory on the host with the same name as the directory which MPI is invoked).

### Run oni-lda-c for topic estimation

The command line used to invoke lda will vary with MPI implementation. Here is a sample command line:

```
mpiexec -n PROCESS_COUNT -f MACHINEFILE ./lda est ALPHA TOPIC_COUNT SETTINGS PROCESS_COUNT INPUT_FILE random outputir
```

where:

- **PROCESS_COUNT** is the total number of processes running on the MPI hosts (the sum of the process counts in the machinefile)
- **MACHINEFILE** is a file containing hosts and their process counts, in the format desecribed in the configuration section above
- **est** Tells lda to estimate the topic model.
- **ALPHA** The alpha parameter for LDA.
- **TOPIC_COUNT** is the number of topics to be used to profile the data.
- **SETTINGS** is a text file containing further settings for LDA, see configuration above
- **INPUT_FILE** contains a representation of the corpus in the format described in **Prepare data for input**
- **outputdir** is the name of a directory in which to store output files

Yes, PROCESS_COUNT appears twice right now (once for MPI and once for LDA).

Example:
```
[user@system ~]$ cd oni-lda-c 
[user@system ~]$  mpiexec -n 20 -f machinefile ./lda est 2.5  12 settings.txt 20 corpus20151031.txt random results20151031
```


## oni-lda-c output


There are two files that will appear in the output directory:

**final.beta** in which each row corresponds to a topic and each row contains the natural logarithm of the probability of each word given that topic.

For example,  if in topic 1, the probability of word 1 were 0.1, the probability of word 2 were 0.89 and the probability of word 3 were 0.01,
then the first line of final.beta would be  −2.302585093 −0.116533816 −4.605170186

**final.gamma** in which each row corresponds to a document and each row contains the unnormalized probabilities of the topics for that document.

For example, if the first line of final.gamma were
0.0224561925 0.0224561925 0.0224561925 4.0224561925 4.0224561925
then document 1 would a topic mix of approximately 0.3% topic 1, 0.3% topic 2, 0.3% topic 3, 49.6% topic 4 and 49.6% topic 5.


## Licensing

oni-lda-c is licensed under the GNU Lesser Public License, version 2.1

## Contributing

Because of the critical position that oni-lda-c holds in the Open-Network-Insight system, contributions to oni-lda-c will be carefully vetted.

## Issues

Report issues at the [Open-Network-Insight issues page](https://github.com/Open-Network-Insight/open-network-insight/issues).

## Maintainers


[Ricardo Barona](https://github.com/rabarona)

[Nathan Segerlind](https://github.com/NathanSegerlind)

[Everardo Lopez Sandoval](https://github.com/EverLoSa)

