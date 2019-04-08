#define _GNU_SOURCE
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <math.h>


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DEFINES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

#define	ERROR_CHECK( cond , str )				\
				do{								\
					if( cond )					\
					{							\
						printf(str);			\
						exit(0);				\
					}							\
				}while(0);

#define printf 

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONSTANTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

pthread_t* tid = NULL;
const int Amount_of_param = 2;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ STRUCTURES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

struct interval_bounds_t
{
	double left;
	double right;
};

struct ht_info
{
	unsigned int* cpu;
	unsigned int max;
	unsigned int amount;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void* doSomeThing(void *arg);
unsigned int* get_proc_info(unsigned int* amount);
unsigned int get_amount_of_htreads(unsigned int* proc_info, unsigned int amount);
struct ht_info* converter(unsigned int* proc_info, unsigned int amount_of_cpu, unsigned int amount_of_ht);
int get_num_function(char *str);
void print_affinity();

//
// 1 / x^2 inegral 0.5 - 2
//

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//    MAIN.   MAIN?   MAIN!    //
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
int main(int argc, char** argv)
{
	ERROR_CHECK((argc != Amount_of_param), "< ERROR > Invalid amount of args!\n");
	int num = get_num_function(argv[1]);
	printf(">>> Num = %d\n", num);

	ERROR_CHECK((num == 0), "No treads = no work :-] \n");
		

    print_affinity();
	unsigned int proc_amount = 0;
	unsigned int* proc_info = get_proc_info(&proc_amount);
	unsigned int ht_amount = get_amount_of_htreads(proc_info, proc_amount) + 1;
	struct ht_info* ht_info = converter(proc_info, proc_amount, ht_amount);
	unsigned int cur_proc = 0;
	unsigned int cur_ht = 0;
	struct interval_bounds_t* bounds = (struct interval_bounds_t*) calloc(num, sizeof(struct interval_bounds_t));
	ERROR_CHECK((bounds == NULL), "< ERROR > Can't allocate memory!\n");

   /* Initialize and set thread detached attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	double d_interval = 1.5 / num;

	printf(">>> ht_amount = %d\n", ht_amount);
	fflush(stdout);

	cpu_set_t cur_set;
	CPU_ZERO(&cur_set);
	CPU_SET(0, &cur_set);
	sched_setaffinity(0, sizeof(cpu_set_t), &cur_set);

	tid = (pthread_t*) calloc(num, sizeof(pthread_t));
    int err;

    int i = 0;
    while(i < num)
    {
		cur_ht = i % ht_amount;
		cur_proc = ht_info[cur_ht].cpu[ht_info[cur_ht].amount % ht_info[cur_ht].max];
		ht_info[cur_ht].amount++;
		printf(">>> Lock %d thread on %u cpu\n", i, cur_proc);
		fflush(stdout); // worked!
		CPU_ZERO(&cur_set);
		CPU_SET(cur_proc, &cur_set);
		sched_setaffinity(0, sizeof(cpu_set_t), &cur_set);

		bounds[i].left = 0.5 + (i * d_interval);
		bounds[i].right = 0.5 + ((i + 1) * d_interval);
		//bounds.cpu_num = ;
        err = pthread_create(&(tid[i]), &attr, doSomeThing, (void*)&bounds[i]);

        ERROR_CHECK((err != 0),"< ERROR > Can't create thread! \n");
        printf(">>> Thread %d created successfully\n", i);

        i++;
    }

	i = 0;
	void *res = NULL;
	int join_ret = 0;
	double final = 0;

	while(i < num)
	{
    	join_ret = pthread_join(tid[i], &res);
		printf("@@@ i = %d, res = %f\n", i, *((double*) res));    
		final = final + *((double*) res);

		ERROR_CHECK((join_ret != 0), "< ERROR > Can't return from thread!\n")

	   // printf("Joined with thread %d; returned value was %s\n", tinfo[tnum].thread_num, (char *) res);

	    free(res);      /* Free memory allocated by thread */
		i++;
    }

#undef printf 
	printf("\n\n\n FINNALY! ANSWER = %f!\n\n", final);
#define printf 

//    sleep(1);
	pthread_attr_destroy(&attr);
	free(ht_info);
	free(bounds);
	free(proc_info);
	free(tid);
    return 0;
}


struct ht_info* converter(unsigned int* proc_info, unsigned int amount_of_cpu, unsigned int amount_of_ht)
{
	struct ht_info* ret = (struct ht_info*) calloc(amount_of_ht, sizeof(struct ht_info));

	unsigned int cur_cpu = 0;
	unsigned int cur_ht = 0;
	unsigned int cur_len = 0;

	while(cur_ht < amount_of_ht)
	{
		cur_cpu = 0;
		cur_len = 0;

		while(cur_cpu < amount_of_cpu)
		{
			if(proc_info[cur_cpu++] == cur_ht)
				cur_len++;
		}

		ret[cur_ht].cpu = (unsigned int*) calloc(cur_len, sizeof(unsigned int));
		ret[cur_ht].max = cur_len;
		ret[cur_ht].amount = 0;

		cur_cpu = 0;

		while(cur_cpu < amount_of_cpu)
		{
			if(proc_info[cur_cpu] == cur_ht)
			{
				ret[cur_ht].cpu[ret[cur_ht].amount] = cur_cpu;
				printf("<0> <0> ret[%u].cpu[%u] = %u \\\\max = %u;\n", cur_ht, ret[cur_ht].amount, cur_cpu, ret[cur_ht].max);
				ret[cur_ht].amount++;
			}

			cur_cpu++;
		}

		ret[cur_ht].amount = 0;
		cur_ht++;
	}

	return ret;
}


unsigned int get_amount_of_htreads(unsigned int* proc_info, unsigned int amount)
{
	unsigned int i = 0;
	unsigned int max = 0;

	while(i < amount)
	{
		if(proc_info[i] > max)
			max = proc_info[i];		

		i++;
	}
	
	return max;
}


unsigned int* get_proc_info(unsigned int* amount)
{
	char str[100] = {0};
	unsigned int proc_amount = 0;
	unsigned int real_proc_amount = 0;
	unsigned int cur_proc = 0;
	int i = 0;

	FILE *pf = popen("grep \"processor\" /proc/cpuinfo", "r");
	ERROR_CHECK((pf == NULL), "< ERROR > Can't do popen first!\n");
	while(fgets(str, 100 , pf) != NULL)
	{
		fprintf(stdout, "%s", str);
		proc_amount++;
	}
	ERROR_CHECK((pclose(pf) != 0), "< ERROR > Can't close pf first!\n");
	unsigned int* ret = (unsigned int*) calloc(proc_amount, sizeof(unsigned int));
	*amount = proc_amount;


	pf = popen("grep \"core id\" /proc/cpuinfo", "r");
	ERROR_CHECK((pf == NULL), "< ERROR > Can't do popen second!\n");
	while(fgets(str, 100 , pf) != NULL)
	{
		fprintf(stdout, "%s", str);

		i = 0;
		while(str[i] != ':')
			i++;

		i = i + 2;

		ret[cur_proc] = get_num_function(str + i);

		cur_proc++;
	}
	ERROR_CHECK((pclose(pf) != 0), "< ERROR > Can't close pf second!\n");

	i = 0;
	while(i < proc_amount)
	{
		printf(">> tread num of %d proc = %u\n", i, ret[i]);
		i++;
	}

	//*amount = get_amount_of_htreads(ret, proc_amount);

	return ret;
}



void print_affinity() 
{
    cpu_set_t mask;
    long nproc, i;

    if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) 
	{
        perror("sched_getaffinity");
        assert(false);
    } 
	else 
	{
        nproc = sysconf(_SC_NPROCESSORS_ONLN);
        printf("sched_getaffinity = ");
        for (i = 0; i < nproc; i++) 
		{
            printf("%d ", CPU_ISSET(i, &mask));
        }
        printf("\n");
    }
}


void* doSomeThing(void *arg)
{
    unsigned long i = 0;
    pthread_t id = pthread_self();

	double left = ((struct interval_bounds_t*) arg)->left;
	double right = ((struct interval_bounds_t*) arg)->right;
	double cur_x = left;
	double step = 0.00000001;
	double* ret = (double*) calloc(1, sizeof(double));
//	*ret = (left + right) / 2;
	*ret = 0;

	while(cur_x < right)
	{
		if(cur_x * cur_x != 0)
			*ret = *ret + (step / (cur_x * cur_x)); // step * y(x) // y(x) = 1/x^2
		cur_x = cur_x + step;
	}

    printf(">>> %lu id in %d\n>>> left = %f, right = %f\n>>>-------<<<\n", id, sched_getcpu(), left, right);
	

//    for(i = 0; i < (0xFFFFFFFF); i++);
//	    sleep(1);

    return (void*) ret;
}


int get_num_function(char *str)
{
	char *pend;
	long int num = strtol(str, &pend, 0);

	ERROR_CHECK((*pend != '\0' && *pend != '\n'), "< ERROR > Invalid num in arg!\n");
	ERROR_CHECK((errno == ERANGE), "< ERROR > Num is very big!\n");

	return num;
}


