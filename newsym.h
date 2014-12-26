#ifndef NEWSYM_H
#define NEWSYM_H

typedef struct NewSym {
  char *sym;
  float val;
  struct NewSym *next; //pointer to the next symbol table
  struct NewSym *list; //list of the variables for that scope
} NewSym;

extern NewSym* globaltable;

NewSym* insertNewvar(char* string, NewSym* localtable);

NewSym* symLookup(char* string, NewSym* local);

NewSym* insertNewtable(NewSym* outerscope); //can be NULL

#endif /* NEWSYM_H */
