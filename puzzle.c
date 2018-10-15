#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <math.h>


/**
* READ THIS DESCRIPTION
*
* node data structure: containing state, g, f
* you can extend it with more information if needed
*/
typedef struct node{
	int state[16];
	int g;
	int f;
	int prev_move;
} node;

/**
* Global Variables
*/

// used to track the position of the blank in a state,
// so it doesn't have to be searched every time we check if an operator is applicable
// When we apply an operator, blank_pos is updated
int blank_pos;

// Initial node of the problem
node initial_node;

// Statistics about the number of generated and expendad nodes
unsigned long generated;
unsigned long expanded;


/**
* The id of the four available actions for moving the blank (empty slot). e.x.
* Left: moves the blank to the left, etc.
*/

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

#define INV_LEFT 1
#define INV_RIGHT 0
#define INV_UP 3
#define INV_DOWN 2

/*
* Helper arrays for the applicable function
* applicability of operators: 0 = left, 1 = right, 2 = up, 3 = down
*/
int ap_opLeft[]  = { 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 };
int ap_opRight[]  = { 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 };
int ap_opUp[]  = { 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
int ap_opDown[]  = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 };
int *ap_ops[] = { ap_opLeft, ap_opRight, ap_opUp, ap_opDown };


/* print state */
void print_state( int* s )
{
	int i;

	for( i = 0; i < 16; i++ )
	printf( "%2d%c", s[i], ((i+1) % 4 == 0 ? '\n' : ' ') );
}

void printf_comma (long unsigned int n) {
	if (n < 0) {
		printf ("-");
		printf_comma (-n);
		return;
	}
	if (n < 1000) {
		printf ("%lu", n);
		return;
	}
	printf_comma (n/1000);
	printf (",%03lu", n%1000);
}

/* Calculates the xpos for a given state */
int xpos(int state){
	return state%4;
}

int ypos(int state){
	return state/4;
}

/* return the sum of manhattan distances from state to goal */
int manhattan( int* state )
{
	int sum = 0;
	int x_dist, y_dist;
	int i=0;

	while( i<16 ){
		if( state[i] != 0 ){
			x_dist = abs( xpos(i) - xpos(state[i]) );
			y_dist = abs( ypos(i) - ypos(state[i]) );
			sum = sum + (x_dist + y_dist);
		}
		i++;
	}
	return( sum );
}


/* return 1 if op is applicable in state, otherwise return 0 */
int applicable( int op )
{
	return( ap_ops[op][blank_pos] );
}


/* apply operator */
void apply( node* n, int op )
{
	int t;

	//find tile that has to be moved given the op and blank_pos
	t = blank_pos + (op == 0 ? -1 : (op == 1 ? 1 : (op == 2 ? -4 : 4)));

	//apply op
	n->state[blank_pos] = n->state[t];
	n->state[t] = 0;

	//update blank pos
	blank_pos = t;
}

int min_threshold(int threshold, int* newThreshold){
	if(threshold < *newThreshold){
		return threshold;
	}
	return *newThreshold;
}

int prevent_reset(int op,int prev_op){

	if(prev_op==LEFT && op==RIGHT){
		return 0;
	}
	if(prev_op==RIGHT && op==LEFT){
		return 0;
	}
	if(prev_op==UP && op==DOWN){
		return 0;
	}
	if(prev_op==DOWN && op==UP){
		return 0;
	}
	return 1;
}

void move_back(node* node, int prev){
	if(node->prev_move == 5){
		return;
	}
	else{
		if(node->prev_move == 0 || node->prev_move == 2){
			apply(node,node->prev_move+1);
			node->prev_move = prev;
		}
		else{
			apply(node,node->prev_move-1);
			node->prev_move = prev;
		}
	}
}


/* Recursive IDA */
node* ida( node* node, int threshold, int* newThreshold )
{

	struct node * r = NULL;
	int last_move = node->prev_move;
	int i;

	for(i=0;i<4;i++){ /* line 1 */

		if(applicable(i))
		{
			generated++;
			node->prev_move = i;
			apply(node,i); /* line 2 */
			node->g = node->g+1; /* line 3*/
			node->f = node->g + manhattan(node->state); /* line 4*/
			if(node->f > threshold){ /* line 5 */
				*newThreshold = min_threshold(node->f,newThreshold); /* line 6*/
			}
			else{ /* line 7*/
				if(manhattan(node->state)==0){ /* line 8 */
					return node; /* line 9 */
				}
				r = ida(node,threshold,newThreshold); /* line 10 */
				if(r!=NULL){ /* line 11 */
					return r; /* line 12 */
				}
			}
			move_back(node,last_move);
			node->g = node->g-1;
		}
	}

	return( NULL ); /* line 13 */
}


/* main IDA control loop */
int IDA_control_loop(  ){
	node* r = NULL;
	node* node_state = malloc(sizeof(node));
	int threshold;
	int newThreshold;

	generated = 0;
	expanded = 0;

	initial_node.f = threshold = manhattan( initial_node.state ); /* line 1 */

	printf( "Initial Estimate = %d\nThreshold = ", threshold );
	while(r==NULL){ /* line 2 */
		newThreshold = INT_MAX; /* line 3 */
		memcpy(node_state->state, initial_node.state, sizeof(node_state->state)); /* line 4 */
		node_state->g = 0; /* line 5 */
		r = ida(node_state,threshold,&newThreshold); /* line 6 */
		if(r==NULL){ /* line 7 */
			threshold = newThreshold; /* line 8 */
		}
	}
	if(r)
	return r->g;
	else
	return -1;
}


static inline float compute_current_time()
{
	struct rusage r_usage;

	getrusage( RUSAGE_SELF, &r_usage );
	float diff_time = (float) r_usage.ru_utime.tv_sec;
	diff_time += (float) r_usage.ru_stime.tv_sec;
	diff_time += (float) r_usage.ru_utime.tv_usec / (float)1000000;
	diff_time += (float) r_usage.ru_stime.tv_usec / (float)1000000;
	return diff_time;
}

int main( int argc, char **argv )
{
	int i, solution_length;

	/* check we have a initial state as parameter */
	if( argc != 2 )
	{
		fprintf( stderr, "usage: %s \"<initial-state-file>\"\n", argv[0] );
		return( -1 );
	}


	/* read initial state */
	FILE* initFile = fopen( argv[1], "r" );
	char buffer[256];

	if( fgets(buffer, sizeof(buffer), initFile) != NULL ){
		char* tile = strtok( buffer, " " );
		for( i = 0; tile != NULL; ++i )
		{
			initial_node.state[i] = atoi( tile );
			blank_pos = (initial_node.state[i] == 0 ? i : blank_pos);
			tile = strtok( NULL, " " );
		}
	}
	else{
		fprintf( stderr, "Filename empty\"\n" );
		return( -2 );

	}

	if( i != 16 )
	{
		fprintf( stderr, "invalid initial state\n" );
		return( -1 );
	}

	/* initialize the initial node */
	initial_node.g=0;
	initial_node.f=0;
	initial_node.prev_move = 5;

	print_state( initial_node.state );


	/* solve */
	float t0 = compute_current_time();

	solution_length = IDA_control_loop();

	float tf = compute_current_time();

	/* report results */
	printf( "\nSolution = %d\n", solution_length);
	printf( "Generated = ");
	printf_comma(generated);
	printf("\nExpanded = ");
	printf_comma(expanded);
	printf( "\nTime (seconds) = %.2f\nExpanded/Second = ", tf-t0 );
	printf_comma((unsigned long int) expanded/(tf+0.00000001-t0));
	printf("\n\n");

	/* aggregate all executions in a file named report.dat, for marking purposes */
	FILE* report = fopen( "report.dat", "a" );

	fprintf( report, "%s", argv[1] );
	fprintf( report, "\n\tSoulution = %d, Generated = %lu, Expanded = %lu", solution_length, generated, expanded);
	fprintf( report, ", Time = %f, Expanded/Second = %f\n\n", tf-t0, (float)expanded/(tf-t0));
	fclose(report);

	return( 0 );
}
