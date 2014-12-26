#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scanner.h"

/* array of reserved words and their corresponding (int) value in the enum */
static struct reserved {
  char *str;
  int tok;
} reserved[] = {
  {"OR", OR_},
  {"AND", AND_},
  {"NOT", NOT_,},
  {"IF", IF_},
  {"THEN", THEN_},
  {"ELSIF", ELSIF_},
  {"ELSE", ELSE_},
  {"FI", FI_},
  {"WHILE", WHILE_},
  {"DO", DO_},
  {"OD", OD_},
  {"HOME", HOME_},
  {"PENUP", PENUP_},
  {"PENDOWN", PENDOWN_},
  {"FORWARD", FORWARD_},
  {"RIGHT", RIGHT_},
  {"LEFT", LEFT_},
  {"PUSHSTATE", PUSHSTATE_},
  {"POPSTATE", POPSTATE_}
};

int lineno = 1;

#define MAXTOK 100   /* warning: no overflow checking implemented */

/*
 * Return the next token from stdin
 * return 0 if there is no more token
 * 
 * */
int nextToken(LVAL *lval) {
  char tok[MAXTOK];
  int i;
  int c;

 top:
  /*
   * Eat whitespace.
   */
  while (strchr(" \t\r\n", c = getchar()) != NULL)
    if (c == '\n') lineno++;

  /*
   * Check for end of input.
   */
  if (c == EOF) return 0;  /* no more tokens */

  /*
   * If c is digit, then scan real number.
   */
  if ('0' <= c && c <= '9') {
    i = 0;
    tok[i++] = c;
    while ('0' <= (c = getchar()) && c <= '9')
      tok[i++] = c;
    if (c == '.') {
      tok[i++] = c;
      while ('0' <= (c = getchar()) && c <= '9')
        tok[i++] = c;
    }
    tok[i] = '\0';

    /* need to push the last c back to stdin */
    ungetc(c, stdin);
    lval->f = atof(tok);
    return REAL_;
  }

  /*
   * If c alphabetic, scan reserved word or user defined identifier.
   */
  if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
    i = 0;
    tok[i++] = c;
    while (('a' <= (c = getchar()) && c <= 'z') || ('A' <= c && c <= 'Z') || 
           c == '_' || ('0' <= c && c <= '9'))
      tok[i++] = c;
    tok[i] = '\0';

    /* need to push the last c back to stdin */
    ungetc(c, stdin);

    for (i = 0; i < sizeof(reserved)/sizeof(struct reserved); i++)
      if (strcmp(reserved[i].str, tok) == 0)
        return reserved[i].tok;

    /* allocate the token in the heap */
    lval->s = strdup(tok);
    return IDENT_;
  }

  /*
   * Assignment operator.
   */
  if (c == ':') {
    if ((c = getchar()) != '=') {
      fprintf(stderr, "%d: bogus lexical element ':'!\n", lineno);
      exit(-1);
    }
    return ASSIGN_;
  }

  /*
   *  '<=' or '<>' or '<'
   */ 
  if (c == '<') {
    if ((c = getchar()) == '=') {
      return LE_;
    } else if (c == '>') {
      return NE_;
    } else {
      ungetc(c, stdin);
      return '<';
    }
  }
      
  /*
   *  '>=' or '>'
   */ 
  if (c == '>') {
    if ((c = getchar()) == '=') {
      return GE_;
    } else {
      ungetc(c, stdin);
      return '>';
    }
  }

  /*
   * Other single character token.
   */
  if (strchr("+-*/()=", c) != NULL)
    return c;

  /*
   * Comment.
   */
  if (c == '#') {
    while ((c = getchar()) != EOF && c != '\n')
      ;
    lineno++;
    goto top;
  }

  /*
   * Lexical error if we get here!
   */
  fprintf(stderr, "%d: unknown lexical element!\n", lineno);
  exit(-1);
}

#ifdef TEST_SCANNER

int main(void) {

  while (1) {
    LVAL lval;
    int lookahead = nextToken(&lval);
    if (lookahead == 0)
      break;
    if (lookahead < 128)  /* single char token */
      printf("%c\n", lookahead);
    else {
      switch(lookahead) {
      case IDENT_: printf("IDENT (%s)\n", lval.s); break;
      case ASSIGN_: printf("ASSIGN\n"); break;
      case REAL_: printf("REAL (%f)\n", lval.f); break;
      case NE_:  printf("NE\n"); break;
      case LE_: printf("LE\n"); break;
      case GE_: printf("GE\n"); break;
      case OR_:  printf("OR\n"); break;
      case AND_:  printf("AND\n"); break;
      case NOT_: printf("NOT\n"); break;
      case WHILE_:  printf("WHILE\n"); break;
      case DO_:  printf("DO\n"); break;
      case OD_: printf("OD\n"); break;
      case IF_:  printf("IF\n"); break;
      case THEN_:  printf("THEN\n"); break;
      case ELSIF_:  printf("ELSIF\n"); break;
      case ELSE_:  printf("ELSE\n"); break;
      case FI_: printf("FI\n"); break;
      case HOME_:  printf("HOME\n"); break;
      case PENUP_:  printf("PENUP\n"); break;
      case PENDOWN_:  printf("PENDOWN\n"); break;
      case FORWARD_:  printf("FORWARD\n"); break;
      case RIGHT_:  printf("RIGHT\n"); break;
      case LEFT_:  printf("LEFT\n"); break;
      case PUSHSTATE_:  printf("PUSHSTATE\n"); break;
      case POPSTATE_: printf("POPSTATE\n"); break;
      default: printf("unknown-token\n");
      }
    }
  }
  return 0;
}

#endif /* TEST_SCANNER */
