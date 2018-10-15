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


void reverse(node*n,int prev_op){
	int op;
	if(prev_op==LEFT){
		op=RIGHT;
	}
	if(prev_op==RIGHT){
		op=LEFT;
	}
	if(prev_op==UP){
		op=DOWN;
	}
	if(prev_op==DOWN){
		op=UP;
	}
	apply(n,op);
}

int min_threshold(int nodef, int* newThreshold){
	if(nodef < *newThreshold){
		return nodef;
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
	else return 1;
}

/* Recursive IDA */
node* ida( node* node, int threshold, int* newThreshold )
{

	struct node * r = NULL;
	int prev_f, prev_g,prev_move,manhattan_val,i;
	prev_move = node->prev_move;

	for(i=0;i<4;i++){
		if(applicable(i)==1 && prevent_reset(i,prev_move)==1)
		{
			generated++;
			/* Apply the action */
			node->prev_move = i;
			printf("===preapply=====\n");
			printf("===Blank_pos %d =====\n",blank_pos);
			print_state(node->state);
			apply(node,i);
			printf("===post apply=====\n");
			printf("===Blank_pos %d =====\n",blank_pos);
			print_state(node->state);
			manhattan_val = manhattan(node->state);
			prev_g = node->g;
			prev_f = node->f;
			node->g = node->g+1;
			node->f = node->g + manhattan_val;

			if(node->f > threshold){
				*newThreshold = min_threshold(node->f,newThreshold);
			}
			else{
				/* if heuristic is 0 return solution */
				if(manhattan_val==0){
					return node;
				}
				/* if not repeat IDA with new state */
				printf("\n====Recursive Call====\n");
				r = ida(node,threshold,newThreshold);
				if(r){
					return r;
				}
			}
		node->g = prev_g;
		node->f = prev_f;
		printf("===pre reverse %d:%d=====\n",prev_move,blank_pos);
		reverse(node,i);
		printf("===reversed %d:%d=====\n",prev_move,blank_pos);
		print_state(node->state);
		}
	}
	return( NULL );
}


/* main IDA control loop */
int IDA_control_loop(  ){
	node* r = NULL;
	node* node_state = &initial_node;
	int threshold;
	int newThreshold;

	/* initialize statistics */
	generated = 0; /* nodes created in the search  */
	expanded = 0; /*  */

	/* compute initial threshold B */
	initial_node.f = threshold = manhattan( initial_node.state );

	printf( "Initial Estimate = %d\nThreshold = ", threshold );
	while(!r){
		newThreshold = INT_MAX;
		node_state = &initial_node;
		node_state->g = 0;
		//printf("\n====new loop=====\n");
		r = ida(node_state,threshold,&newThreshold);
		if(!r){
			threshold = newThreshold;
			//printf("\n%d\n",threshold);
		}
			print_state(node_state->state);
			if(threshold > 100){
				exit(EXIT_FAILURE);
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


