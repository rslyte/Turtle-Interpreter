#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"newsym.h"

NewSym* globaltable = NULL;

NewSym* insertNewvar(char* string, NewSym* localtable){ 
  int result;	
  NewSym* index = localtable;
  NewSym* prev;
 
  while (index != NULL){
	prev = index;
	
	if ((result = strcmp(string, index->sym)) == 0){
		fprintf(stderr,"Variable was initialized when it was already initialzed prior in same scope. Program exit\n");
		exit(-1);
	}
	index = index->list;
  }
  	 
  index = (NewSym*) malloc(sizeof(NewSym));
  index->sym = (char*) strdup(string);
  index->val = 0; //all variables are initialized to 0	
  index->next = NULL;
  index->list = NULL;
  
  prev->list = index;
  //printf("string '%s' appended.\n", index->sym);
  return index;
}

NewSym* symLookup(char* string, NewSym* local){
  NewSym* index = NULL;
  NewSym* globalindex = local;	 
  int result;

  while (globalindex != NULL){
	//printf("made in it inside first while loop\n");
	for(index = globalindex; index != NULL; index = index->list){
		//printf("inside for-loop, compare %s with %s from the node.\n", string, index->sym);
		if ((result = strcmp(string, index->sym)) == 0){
			return index;
		}
		//printf("%s and %s compare was %i\n", string, index->sym, result);
	}
  	globalindex = globalindex->next;
  }
  
  for (globalindex = globaltable; globalindex != NULL; globalindex = globalindex->list){
	//printf("globaltable now, passed in %s compared with %s\n", string, globalindex->sym);
  	if ((result = strcmp(string, globalindex->sym)) == 0){
		return globalindex;		
	}
	//printf("%s and %s compare was %i\n", string, globalindex->sym, result);
  }
  //return NULL if there wasn't even a var int global table
  fprintf(stderr,"You are trying to use variable %s that hasn't been initialized. Program exit\n", string); 
  exit(-1);
  return NULL; //just to keep the compiler happy
}

NewSym* insertNewtable(NewSym* outerscope){
   
  NewSym* table;
  
  table = (NewSym*) malloc(sizeof(NewSym));
  table->sym = (char*) strdup("headoflist");
  table->val = 999;
  table->next = outerscope; //reference to the next scope up
  table->list = NULL;
  return table;
}





