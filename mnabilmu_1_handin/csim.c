/*  AndrewID : mnabilmu
 *  Name     : Mohd. Nabil Murshed
 *
 *
*/

/*
      ************************************
      |       Importing Libraries        |
      ************************************
*/
#include "stdio.h"
#include "stdlib.h"
#include "cachelab.h"
#include "getopt.h"
#include "unistd.h"
#include "limits.h"

int s , E , b ;

int v_flag ;
int globalage = 0;
int miss_count, hit_count, eviction_count = 0 ;

/*
      ************************************
      |       Defining Structures        |
      ************************************
*/
typedef struct
{
  int valid;
  int tag;
  int age;
}cache_line;

typedef struct
{
  int lcount;
  cache_line *lines;
}cache_set;

typedef struct
{
  int scount;
  cache_set *sets;
}cache;


/*
      *********************************************
      |       Allocating and freeing cache        |
      *********************************************
*/

void init(cache **c)
{
  int i;
  *c = (cache*)malloc(sizeof(cache));
  (*c)->scount = 1<<s;
  (*c)->sets = (cache_set*)malloc(sizeof(cache_set) * (1<<s));
  for(i=0;i<(*c)->scount;i++)
  {
    (*c)->sets[i].lcount = E;
    (*c)->sets[i].lines = (cache_line*)calloc ( E , sizeof(cache_line) );
  }
}

void free_cache(cache* c)
{
  int i;
  for(i=0 ; i<(c->scount) ; i++)
  {
    free(c->sets[i].lines);
  }
  free(c->sets);
  free(c);
}

/*
      **********************************************
      |         Breaking down the address          |
      **********************************************
*/


int bitMask(int highbit, int lowbit)
{
  int ff=~0;
  return ((((ff<<highbit)<<1)^ff)>>lowbit)<<lowbit;
}

int set_bits(int addr)
{
  return (addr & bitMask(s+b-1, b)) >> b;  
}

int tag_bits(int addr)
{
  return (addr ) >> (b+s);
}

/*
      *********************************************
      |             Cache operations              |
      *********************************************
*/

void upload(cache_set *curr_set, int t_b)
{
  int minage=INT_MAX;
  int i = 0;
  cache_line *ln;
  for(i=0; i<(curr_set->lcount); i++)
  {
    if(curr_set->lines[i].valid == 0)
    {
      curr_set->lines[i].valid = 1;
      curr_set->lines[i].tag = t_b;
      curr_set->lines[i].age = globalage;
      globalage++;
      return;
    }
  }
  
  //Gets the min age and a pointer to that line
  for(i=0; i<(curr_set->lcount); i++)
  {
    if(curr_set->lines[i].age < minage)
    {
      minage = curr_set->lines[i].age;
      ln = &(curr_set->lines[i]);
    }
  }

  //If a min age line exists, evict it and replace
  if(ln != NULL)
  {
    //globalage++;
    ln->valid = 1;
    ln->tag = t_b;
    ln->age = globalage;
    globalage++;
    eviction_count++;
    if(v_flag) printf("eviction ");
  }
  return;
}

void lookup(cache *c, int t_b, int s_b)
{
  int i;
  cache_set curr_set = c->sets[s_b];
  for(i=0; i<(curr_set.lcount); i++)
  {
    if( (curr_set.lines[i].valid == 1) & (curr_set.lines[i].tag == t_b) )
    {
      hit_count++;
      curr_set.lines[i].age = globalage;
      globalage++;
      if(v_flag) printf("hit ");
      return;
    }
  }

  //Executes this if miss occurs
  miss_count++;
  if(v_flag) printf("miss ");
  upload( &(c->sets[s_b]), t_b);
  return;
}

void run(cache *c, int addr)
{
  lookup( c, tag_bits(addr), set_bits(addr));
}


/********************** MAIN **********************/
int main(int argc, char **argv)
{
  int opt;
  FILE *tfile;
  char id;
  int addr;
  cache *mycache;
 
  while(-1 != (opt = getopt(argc,argv,"hvs:E:b:t:")) )
  {
    switch(opt)
    {

    case 'h' :
	printf("Correct Usage : %s [-hv] -s <s> -E <E> -b <b> -t <tracefile> \n", argv[0]);
	break;
    case 'v' :
      v_flag = 1;
      break;
    case 's' :
      s = atoi(optarg);
      break;
    case 'E' :
      E = atoi(optarg);
      break;
    case 'b' :
      b = atoi(optarg);
      break;
    case 't' :
      tfile = fopen(optarg, "r");
      break;
    default:
      printf("Incorrect args \n");
      printf("Correct Usage : %s [-hv] -s <s> -E <E> -b <b> -t <tracefile> \n", argv[0]);
      break;
    }
  }

  init(&mycache);                              //initialize cache  

  while(fscanf(tfile,"%c %x",&id,&addr) > 0)
  {
    switch(id)
    {
    case 'L' :
      run(mycache,addr);
      break;
    case 'S' :
      run(mycache,addr);
      break;
    case 'M' :
      run(mycache,addr);
      run(mycache,addr);
      break;
    default:
      break;
    }
    if(v_flag)printf("\n");
  }

  printSummary(hit_count, miss_count, eviction_count);
  fclose(tfile);
  free_cache(mycache);

  return 0;
}
