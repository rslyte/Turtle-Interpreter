/*
RYAN SLYTER
CS355
ASSIGNMENT 3
CHANING TURTLE TO STATIC SCOPING
*/
#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include"scanner.h"
#include"newsym.h"
#include"turtle.h"
#include<assert.h>

/***struct and enum declares: NOTE: lval is a pointer to a union of char* / float *************************************/
enum {
	ASSIGN_STMT = 1, WHILE_STMT, IF_STMT, BLOCK_STMT, ACTION_STMT, NUM_EXPR, VAR_EXPR, ADD_EXPR, SUB_EXPR, MULT_EXPR, DIV_EXPR, NEG_EXPR, GE_EXPR, LE_EXPR, NE_EXPR, L_EXPR, G_EXPR, OR_EXPR, AND_EXPR, NOT_EXPR, E_EXPR, ELSIF_STMT};

//NOTE: I added NEG_EXPR - G_EXPR to be able to evaluate boolean expressions

typedef struct Expr {
	int type; /* NUM_EXPR, VAR_EXPR, ADD_EXPR, ... */
	union {
		float num; /* NUM_EXPR */
		NewSym *sym; /* VAR_EXPR */
		
		/* unary operation (NEG_EXPR, ...NOT_EXPR?) */
		struct Expr *unary;

		/* binary operation (ADD_EXPR....SUB_EXPR MULT_EXPR DIV_EXPR) */
		struct{struct Expr *left, *right;} binary;
	}op;
}Expr;

typedef struct Stmt {
	int type; /* ASSIGN_STMT, WHILE STMT, BLOCK_STMT, etc... */
	union {
		struct {NewSym *lval; Expr *rval;} assign_;
		struct {Expr *cond; struct Stmt *body;} while_;
		struct {NewSym *varlist; struct Stmt *list;} block_; //ASSIGNMENT 3: NEED A NEW MEMBER
		struct {Expr *cond; struct Stmt *body; struct Stmt *elseBody; } if_;
		struct {int action; Expr *rval;} action_;
	}s;
	struct Stmt *next; /* link-list field used by block statements */
} Stmt;

/*******************************************function prototypes*************************************************/

void syntax_error(char* message);
void match(int expected);
Expr* factor(NewSym* table);
Expr* createBinaryExpr(int tok, Expr* first, Expr* second);
Expr* term(NewSym* table);
Expr *expr(NewSym* table);
float evalExpr(Expr *expr);
void execActionStmt();
Stmt* createActionStmt(int action, Expr* rval);
Stmt* action_stmt(NewSym* table);
Stmt* assign_stmt(NewSym* table);
int cmp_op();
Expr* cmp(NewSym* table);
Expr* bool_factor(NewSym* table);
Expr* bool_term(NewSym* table);
Expr* bool_stmt(NewSym* table);
Stmt* while_stmt(NewSym* table);
Stmt* if_stmt(NewSym* table);
int stmtPrefix();
Stmt* stmt(NewSym* table);
Stmt* block_stmt(NewSym* table); //note: I destroyed the two free expr/stmt functions I made earlier
void executeStmt();
void stmt_seq();
void program();

/***********************************global variable decs and functions******************************************/

//NewSym* globaltable = insertNewtable(NULL); //assignment3: initialize the new global table
int lookahead = 0;
LVAL *lval;
char* str;
void syntax_error(char* message){
	printf("%s\n", message);
	exit(-1);
}

void match(int expected){
  if (lookahead == expected){
    lookahead = nextToken(lval); 
  }
  else{
    fprintf(stderr,"parsing error, could not match lookahead: %i with expected: %i\n", lookahead, expected);
    exit(-1); //put this in here for debugging purposes
  }
}

//this function creates an expression for float/variable or decends further into the tree
//NOTE case ('-') needs to be changed to create an expression with type NEG_EXPR
Expr* factor(NewSym* table){
	Expr *e;
	switch(lookahead){
		case REAL_:			
			e = (Expr*) malloc (sizeof(Expr));
			e->type = NUM_EXPR;
			e->op.num = lval->f; 
			lookahead = nextToken(lval);
			return e;
			break;

		case IDENT_:
			e = (Expr*) malloc (sizeof(Expr));
			e->type = VAR_EXPR;			
			NewSym* temp = symLookup(lval->s, table); //ASSIGNMENT 3			
			e->op.sym =  temp; //ASSIGNMENT 3
			lookahead = nextToken(lval);
			return e; //return the value associated with the variable according to packet
			break;

	    	case '+':
			match('+');
			return factor(table);
			break;

		case '-':
			match('-');
			e = (Expr*) malloc(sizeof(Expr));
			e->type = NEG_EXPR;
			e->op.unary = factor(table);
			return e;
			break;

		case '(':
			match('(');
			e = expr(table);
			match(')');
			return e;
			break;

		default:
			break;
		}

return NULL;
}

//I made this one, used to create the binary expression
Expr* createBinaryExpr(int tok, Expr* first, Expr* second){
  Expr* toReturn = (Expr*) malloc(sizeof(Expr));
  toReturn->type = tok;
  toReturn->op.binary.left = first;
  toReturn->op.binary.right = second;
  return toReturn;  
}

//handles the term -> {(*|/) factor} production
Expr* term(NewSym* table){
	Expr *e = factor(table);
	while(1) {
		if (lookahead == '*'){
			match('*');
			e = createBinaryExpr(MULT_EXPR, e, factor(table));			
		}else if (lookahead == '/') {
			match('/');
			e = createBinaryExpr(DIV_EXPR, e, factor(table));
		}else {
			break;
		}
	}
	return e;
}

//handles the expr -> term {(+|-) term} production
Expr *expr(NewSym* table){
	Expr *y = term(table);
	while(1) {
		if (lookahead == '+'){
			match('+');
			y = createBinaryExpr(ADD_EXPR, y, term(table));			
		}else if (lookahead == '-') {
			match('-');
			y = createBinaryExpr(SUB_EXPR, y, term(table));
		}else {
			break;
		}
	}
	return y;
}

float evalExpr(Expr *expr){ /* evaluate and expression tree */
	switch(expr->type){
		case NUM_EXPR: 
			return expr->op.num;
		  	break;

		case VAR_EXPR: 
			return expr->op.sym->val; 
			break;

		case ADD_EXPR: 
		 
			return evalExpr(expr->op.binary.left) + evalExpr(expr->op.binary.right);
			break;

	        case SUB_EXPR: 
			return evalExpr(expr->op.binary.left) - evalExpr(expr->op.binary.right);
			break;

	        case MULT_EXPR: 
			return evalExpr(expr->op.binary.left) * evalExpr(expr->op.binary.right);
			break;

	        case DIV_EXPR: 
			return evalExpr(expr->op.binary.left) / evalExpr(expr->op.binary.right);
			break;  

		case NEG_EXPR: 
		        return -evalExpr(expr->op.unary);
			break;

	        case GE_EXPR: 
	                return (evalExpr(expr->op.binary.left) >= evalExpr(expr->op.binary.right))? 1 : 0;
	                break;
	    
                case LE_EXPR:	         
		  return (evalExpr(expr->op.binary.left) <= evalExpr(expr->op.binary.right))? 1:0; 		        
	                break;

	        case NE_EXPR:
                       return (evalExpr(expr->op.binary.left) != evalExpr(expr->op.binary.right))? 1 : 0;
	               break;

	        case L_EXPR:
                       return (evalExpr(expr->op.binary.left) < evalExpr(expr->op.binary.right))? 1 : 0;
	               break;

		case OR_EXPR:
			return ( (evalExpr(expr->op.binary.left) == 1) || (evalExpr(expr->op.binary.right) == 1) )? 1 : 0;
			break;
		
		case AND_EXPR:
			return ( (evalExpr(expr->op.binary.left) == 1) && (evalExpr(expr->op.binary.right) == 1) )? 1 : 0;		

	        case G_EXPR:
                       return (evalExpr(expr->op.binary.left) > evalExpr(expr->op.binary.right))? 1 : 0;
	               break;

	        case NOT_EXPR:
		       return !(evalExpr(expr->op.unary)); 
	               break;
	
		case E_EXPR:
		       return (evalExpr(expr->op.binary.left) == evalExpr(expr->op.binary.right))? 1 : 0;
		       break;

		default:
			break;		
	}			     				      
}

void execActionStmt(Stmt *stmt){
	int action = stmt->s.action_.action;
	float param;
	switch (action){
		case FORWARD_:
			param = evalExpr(stmt->s.action_.rval);
			turtleForward(param);
			break;

		case RIGHT_:
			param = evalExpr(stmt->s.action_.rval);
			turtleRotate(-param);
			break;
		case LEFT_:
			param = evalExpr(stmt->s.action_.rval);
			turtleRotate(param);
			break;

		case HOME_:
			turtleHome();
			break;
		
		case PENUP_:
			turtlePenUp(1);
			break;

		case PENDOWN_:
			turtlePenUp(0);
			break;
		
		case POPSTATE_:
			turtlePopState();
			break;

		case PUSHSTATE_:
			turtlePushState();
			break;	

		default:
			break;

	}
return;	
}

//did in class
Stmt* createActionStmt(int action, Expr* rval){
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ACTION_STMT;
	s->s.action_.action = action;
	s->s.action_.rval = rval;
	return s; //create the actual action statement and return the pointer to it
}


Stmt* action_stmt(NewSym* table){
	int action = lookahead;
	lookahead = nextToken(lval);

	switch(action){    //check what kind of action it is
		case FORWARD_:
			return createActionStmt(action, expr(table));
			break;

		case RIGHT_:
			return createActionStmt(action, expr(table));
			break;

		case LEFT_:
			return createActionStmt(action, expr(table));
			break;

	        case HOME_:
		        return createActionStmt(action, NULL);
	                break;

	        case PENUP_:
		        return createActionStmt(action, NULL);
                        break;

	        case PENDOWN_:
		        return createActionStmt(action, NULL);
		        break;

	        case PUSHSTATE_:
	                return createActionStmt(action, NULL);
	                break;

	        case POPSTATE_:
	                return createActionStmt(action, NULL);
	                break;
		default:
			break;
	}
return NULL;
}
//ASSIGNMENT 3:
//new assign_stmt changed to deal with scoping
//and the new case where in stmt_seq() or block()
//a variable was declared when it supposed to be assigned (so a symlookup not an initialization)
Stmt* assign_stmt(NewSym* table){
	Stmt* e = (Stmt*) malloc(sizeof(Stmt));        
        e->type = ASSIGN_STMT;
	if (lookahead == ASSIGN_){
		//printf("about to symlookup %s\n", lval->s);
		e->s.assign_.lval = symLookup(lval->s,table);
		match(ASSIGN_);
		e->s.assign_.rval = (Expr*) expr(table);
		return e;		
	}
	else {
		assert(lookahead == IDENT_);
		e->s.assign_.lval = symLookup(lval->s, table); 
		match(IDENT_); 
		match(ASSIGN_);
		e->s.assign_.rval = (Expr*) expr(table);
		return e;
	}
}

//function just returns the compare operator
int cmp_op(){
	switch(lookahead){

		case LE_:
			match(LE_);
			return LE_EXPR;
			break;

		case GE_:
			match(GE_);
			return GE_EXPR;
			break;

		case NE_:
			match(NE_);
			return NE_EXPR;
			break;
		
		case '<':
			match('<');
			return L_EXPR;
			break;
		
		case '>':
			match('>');
			return G_EXPR;
			break;

		case '=':
			match('=');
			return E_EXPR;
			break;
		
		default:
			break;
	}
	return 0;
}
//not changed from assignment 1
Expr* cmp(NewSym* table){
	Expr* a = (Expr*) expr(table);
	int operator = cmp_op();
	Expr* b = (Expr*) expr(table);
	Expr* c = createBinaryExpr(operator, a, b);
	//printf("cmp() returned expression operator: %i\n", operator);
	return c;
}
//not changed from assignment 1
Expr* bool_factor(NewSym* table){
	Expr* e;
	switch(lookahead){
	
		case NOT_:
			match(NOT_);
			e = (Expr*) malloc(sizeof(Expr));
			e->type = NOT_EXPR;
			e->op.unary = (Expr*) bool_factor(table);
			return e;
			break;

		case '(':
			match('(');
			e = (Expr*) bool_stmt(table);
			match(')');
			return e;
			break;

		default:
			e = cmp(table);
			return e;
			break;
	}
	return NULL;
}
//unchanged from assignment 1
Expr* bool_term(NewSym* table){
	Expr* e = bool_factor(table);
	while(1){
		if (lookahead == AND_){
			match(AND_);
			e = (Expr*) createBinaryExpr(AND_EXPR, e, bool_factor(table)); //changed on 2/10
		}
		else{break;}
	}
	return e;
}
//This is BOOL from the grammar on the assignment prompt
Expr* bool_stmt(NewSym* table){
        Expr* e = (Expr*) bool_term(table);
	while(1){
		if (lookahead == OR_){
			match(OR_);
			e = (Expr*) createBinaryExpr(OR_EXPR, e, bool_term(table)); //changed 2/10
		}
		else{break;}
	}
	return e;
}       
//creates and returns a while statement
Stmt* while_stmt(NewSym* table){
	match(WHILE_);
	Stmt* e = (Stmt*) malloc(sizeof(Stmt));
	e->type = WHILE_STMT;
	e->s.while_.cond = (Expr*) bool_stmt(table);
	match(DO_);
	e->s.while_.body = (Stmt*) block_stmt(table);
	match(OD_);
	return e;
}        
/*creates an if statement and if there are subsequent
  elseif statements it appends those to e->next like a
  linked list. Puts else block statement in elseBody
  if applicable*/
Stmt* if_stmt(NewSym* table){
	match(IF_);
	Stmt* e = NULL; 
	Stmt* temp = NULL; 
	Stmt* elsifstmt = NULL;
	Stmt* previous = NULL;
	e = (Stmt*) malloc(sizeof(Stmt));
	e->type = IF_STMT;
	e->s.if_.cond = (Expr*) bool_stmt(table); 
	match(THEN_);
	e->s.if_.body =(Stmt*) block_stmt(table);
	e->next = NULL; 
	e->s.if_.elseBody = NULL;
	while (lookahead == ELSIF_){
		match(ELSIF_);
		elsifstmt = (Stmt*) malloc(sizeof(Stmt));
		elsifstmt->type = ELSIF_STMT;
		elsifstmt->s.if_.cond = (Expr*) bool_stmt(table); 
		match(THEN_);
		elsifstmt->s.if_.body = (Stmt*) block_stmt(table);
		elsifstmt->s.if_.elseBody = NULL;
		elsifstmt->next = NULL;
		temp = e->next;              
		while (temp != NULL){        //goal is to walk the ll so that elsif's are in order
  			previous = temp;
			temp = temp->next;
		}
		if (previous != NULL){previous->next = elsifstmt;}
	}
	if (lookahead == ELSE_){
		match(ELSE_);
		e->s.if_.elseBody = (Stmt*) block_stmt(table);
	}
	match(FI_);
return e;
}

//unchanged from assignment 3 except that the ASSIGN_ case was added
Stmt* stmt(NewSym* table){ 
	switch(lookahead){
		case FORWARD_:
			return action_stmt(table);
			break;

		case RIGHT_:			
			return action_stmt(table);
			break;

		case LEFT_:
			return action_stmt(table);
			break;
		
		case HOME_:
			return action_stmt(table);
			break;

		case PENUP_:
			return action_stmt(table);
			break;

		case PENDOWN_:
			return action_stmt(table);
			break;

		case PUSHSTATE_:
			return action_stmt(table);
			break;

		case POPSTATE_:
			return action_stmt(table);
			break;

		case ASSIGN_: //ASSIGNMENT 3 CHANGE: if ASSIGNT_ nxt tok then call modified assign_stmt
	        case IDENT_:                  
		        return assign_stmt(table);
                        break;	

	        case IF_:
	                return if_stmt(table);
	                break;

        	case WHILE_: 
		        return while_stmt(table);
                        break;

		default:
			break; 
		
	}
	return NULL;
}

/*Should return a block statement, has at least 1 stmt and possibly
  a linked list of other statements*/
Stmt* block_stmt(NewSym* table){         
  
  	Stmt* append = NULL; 
	Stmt* e = NULL; 
  	Stmt* previous = NULL; 
  	Stmt* tail = NULL;
	e = (Stmt*) malloc(sizeof(Stmt));
	e->type = BLOCK_STMT;
	e->next = NULL;
	NewSym* newscope = insertNewtable(table);
 	e->s.block_.varlist = newscope;
  	while (lookahead == IDENT_){
		char* string = lval->s;
		match(IDENT_);
		if (lookahead == ASSIGN_){
		break;
		}				
		else{insertNewvar(string, newscope);}
	} 
	
	e->s.block_.list = (Stmt*) stmt(newscope);
	while(lookahead != OD_ && lookahead != FI_ && lookahead != ELSIF_ && lookahead != ELSE_){
		append = (Stmt*) stmt(newscope);
		if (e->next == NULL){
			e->next = append;
		}
		else{
			tail = e->next;
			while(tail->next != NULL){
				tail= tail->next;
			}
			tail->next = append;
		}
	}
	return e;
}	

//Main routine for executing statement trees
void executeStmt(Stmt *stmt) { /* executes a statement tree */ 
  	//printf("execute stmt %d\n", stmt->type);

	int elsiflag = 0;
	Stmt* current = NULL;
	switch(stmt->type) {
		//first 3 cases were done in class/given in assignment prompt
		case ACTION_STMT:
			execActionStmt(stmt);
			break;	

		case ASSIGN_STMT:
			stmt->s.assign_.lval->val = evalExpr(stmt->s.assign_.rval);
			break;

		case WHILE_STMT:
		  	while (evalExpr(stmt->s.while_.cond) != 0){
		        	executeStmt(stmt->s.while_.body);}
			break;
		
		case BLOCK_STMT:	//done 2/8
			executeStmt(stmt->s.block_.list); //execute the one stmt that's in there
			if (stmt->next != NULL){ //make sure there are more appended
				Stmt* current = stmt->next;
				Stmt* prev = NULL;
				while (current != NULL){
					prev = current;					
					current = current->next;
					executeStmt(prev);  //CHANGED THIS 2/10
				}
			}
			break;

		case IF_STMT:			
			if (evalExpr(stmt->s.if_.cond) != 0){
				executeStmt(stmt->s.if_.body);
				break;
			}
			for (current = stmt->next; (current != NULL) && (current->type == ELSIF_STMT); current = current->next){
				if (evalExpr(current->s.if_.cond) != 0){ //go through the l.l. of elseif stmts to find one that is true
					executeStmt(current->s.if_.body); //execute the first 1 that is true and exit for-loop
					elsiflag = 1; //set flag to elseBody wont be executed
					break;
				}	
			}
			if ((elsiflag == 0) && (stmt->s.if_.elseBody != NULL)){ //if first cond and no elsif were executed..
				executeStmt(stmt->s.if_.elseBody); //execute the elseBody and break
				break;
			}

		default:
			break;
		
	}
return;
}

void stmt_seq() {
	
	/*New code for ASSIGNMENT 3, keeping getting vars and if you see a
	  X := 2 or some assignment statement then call assign and execute that
	*/
	int cache;
	globaltable = insertNewtable(NULL);
	while (lookahead == IDENT_){
		char* string = lval->s;
		match(IDENT_);
		if (lookahead == ASSIGN_){
		break;
		}				
		else{insertNewvar(string, globaltable);}
	} 
	
	do {
		Stmt *s = stmt(NULL); /*build syntax for next statement*/
		if (s == NULL){
			fprintf(stderr,"error after statement was called, program exit\n");
			exit(-1);}
		executeStmt(s);   /*execute syntax tree */		
	} while (lookahead != 0); /* keep looping if there are more statements */ //CHANGED FROM STMTPREFIX()
}

void program() {
	stmt_seq();
	if (lookahead != 0){fprintf(stderr,"Extraneous input! Lookahead = %i\n", lookahead);
	  exit(-1);}
}
//MAIN just checks to see if there's tokens, inits turtle and calls program()	
int main(void){
	lval = (LVAL*) malloc(sizeof(LVAL));
	turtleInit();
	lookahead = nextToken(lval);
	if(!lookahead){
		fprintf(stderr, "Nothing to read");
		exit(-1);
	}
	else{
	program();
	turtleDumpImage(stdout);
	}
	return 0;
}	
