#ifndef KANN_H
#define KANN_H

#define KANN_VERSION "r165"

#define KANN_L_IN       1
#define KANN_L_OUT      2
#define KANN_L_TRUTH    3
#define KANN_L_COST     4
#define KANN_L_PIVOT    5

#define KANN_H_TEMP     11
#define KANN_H_DROPOUT  12
#define KANN_H_L2REG    13

#define KANN_C_BIN_CE   1
#define KANN_C_CE       2

#define KANN_RDR_BATCH_RESET     1
#define KANN_RDR_MINI_RESET      2
#define KANN_RDR_READ_TRAIN      3
#define KANN_RDR_READ_VALIDATE   4

#define KANN_MM_DEFAULT 0
#define KANN_MM_SGD     1
#define KANN_MM_RMSPROP 2

#define KANN_MB_DEFAULT 0
#define KANN_MB_CONST   1

#include <stdint.h>
#include "kautodiff.h"

typedef struct {
	int n;
	kad_node_t **v;
	float *t, *g, *c;
} kann_t;

typedef struct {
	float lr;    // learning rate
	float fv;    // fraction of validation data
	int max_mbs; // max mini-batch size
	int max_rnn_len;
	int epoch_lazy;
	int max_epoch;

	float decay;
} kann_mopt_t;

typedef struct {
	int n, epoch;
	short mini_algo, batch_algo;
	float lr;
	float decay;
	float *maux, *baux;
} kann_min_t;

typedef int (*kann_reader_f)(void *data, int action, int max_len, float *x, float *y);
typedef kad_node_t *(*kann_activate_f)(kad_node_t*);

#define kann_n_par(a) (kad_n_var((a)->n, (a)->v))
#define kann_is_hyper(p) ((p)->label == KANN_H_TEMP || (p)->label == KANN_H_DROPOUT || (p)->label == KANN_H_L2REG)

extern int kann_verbose;

#ifdef __cplusplus
extern "C" {
#endif

// common layers
kad_node_t *kann_layer_input(int n1);
kad_node_t *kann_layer_linear(kad_node_t *in, int n1);
kad_node_t *kann_layer_dropout(kad_node_t *t, float r);
kad_node_t *kann_layer_rnn(kad_node_t *in, int n1, kann_activate_f af);
kad_node_t *kann_layer_gru(kad_node_t *in, int n1);
kann_t *kann_layer_final(kad_node_t *t, int n_out, int cost_type);

// basic model allocation/deallocation
void kann_collate_x(kann_t *a);
void kann_set_hyper(kann_t *a, int label, float z);
void kann_delete(kann_t *a);
void kann_delete_unrolled(kann_t *a);

// number of input and output variables
int kann_n_in(const kann_t *a);
int kann_n_out(const kann_t *a);
int kann_n_hyper(const kann_t *a);

// unroll an RNN to an FNN
kann_t *kann_rnn_unroll(kann_t *a, int len);

// train a model
void kann_mopt_init(kann_mopt_t *mo);
void kann_train(const kann_mopt_t *mo, kann_t *a, kann_reader_f rdr, void *data);

// apply a trained model
const float *kann_apply1(kann_t *a, float *x);
void kann_rnn_start(kann_t *a);
void kann_rnn_end(kann_t *a);
float *kann_rnn_apply_seq1(kann_t *a, int len, float *x);

// model I/O
void kann_write_core(FILE *fp, const kann_t *ann);
void kann_write(const char *fn, const kann_t *ann);
kann_t *kann_read_core(FILE *fp);
kann_t *kann_read(const char *fn);

// pseudo-random number generator
void kann_srand(uint64_t seed);
uint64_t kann_rand(void);
double kann_drand(void);
double kann_normal(void);
void kann_shuffle(int n, float **x, float **y, char **rname);
void kann_rand_weight(int n_row, int n_col, float *w);

// kann minimizer
kann_min_t *kann_min_new(int mini_algo, int batch_algo, int n);
void kann_min_delete(kann_min_t *m);
void kann_min_mini_update(kann_min_t *m, const float *g, float *t);
void kann_min_batch_finish(kann_min_t *m, const float *t);

// generic data reader for FNN
void *kann_rdr_xy_new(int n, float frac_validate, int d_x, float **x, int d_y, float **y);
void kann_rdr_xy_delete(void *data);
int kann_rdr_xy_read(void *data, int action, int len, float *x1, float *y1);
void kann_fnn_train(const kann_mopt_t *mo, kann_t *a, int n, float **x, float **y);

#ifdef __cplusplus
}
#endif

#endif
