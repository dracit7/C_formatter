
#define _INC_GLOBAL

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif

// Types Specified
enum Types {
  
  // Token Types
  IDENT = 257,
  INT_CONST, FLOAT_CONST, CHAR_CONST, // Const types
  PLUS, MINUS, MULTI, DIV, MOD, EQUAL, ASSIGN, INCRE, DECRE, // Arithmetic operators
  GREATER, LESS, GEQUAL, LEQUAL, NOT, NEQUAL, AND, OR,  // Comparitive operators
  INT, FLOAT, CHAR, VOID,  // Variable types
  IF, ELSE, WHILE, RETURN, FOR, BREAK, CONTINUE, // Keywords
  SEMI, COMMA, COLON, LPAREN, RPAREN, LBRACE, RBRACE, LBRACK, RBRACK,// Delimiters

  // Node Types
  VAR, FUNC, VARSEQ,        // Two types of IDENT, who are not distinguished during lexical analysis.
  PROGREM,
  FUNCDEF, VARDEF, PARAMLIST, RETURNTYPE, COMPSTATE, ARGSEQ,
  PARAM
};

// Global constants
#define BUFFERSIZE 2048 // Size of the buffer which stores strings.
#define TABLESIZE 128   // Size of the symbolTable which stores tokens.
#define KEYWORD_NUM 11   // Number of keywords in tokenTable.
#define NONE -2

// Global variables
float tokenValue = NONE; // The value of the Token
int lineSerial = 1;
char buffer[BUFFERSIZE]; // Store all identifiers' names linearly in a buffer.
int bufferPtr = 0;       // Point to the next byte of the end of the buffer's used space.
FILE *filePtr;
int tokenPtr = 0;        // This is the pointer for grammatical analysis.
char preBuffer[BUFFERSIZE]; // Buffer for preprocession.

// Util Functions
int isDigit(int); // Judge if a character represents a number
int isAlpha(int); // Judge if a character represents a letter 
int strCmp(char*, char*); // Enhanced strcmp, able to compare string variables.
char* strCpy(char*, const char*); // Enhanced strcpy, returns the end of s1

// Error Handling
void error(char*);      // Report an error and the line serial at present, then exit
void exception(char*);  // Throw an exception and exit

// SymbolTable
int symPtr = 0;  // The present size of symbolTable

typedef struct {
  char* content;
  int type;
} token;

token tokenTable[TABLESIZE];

int find(char*);         /* Returns the index of target token in tokenTable
                            If not exist, return -1 instead.*/
int insert(char*, int);  // Insert a token into tokenTable, returning its index.

token reservedWords[] = {
  "if", IF,
  "else", ELSE,
  "while", WHILE,
  "for", FOR,
  "return", RETURN,
  "break", BREAK,
  "continue", CONTINUE,
  "int", INT,
  "float", FLOAT,
  "char", CHAR,
  "void", VOID,
  // ...
  0, 0
};                       // Keywords in C, they should not be regarded as IDENT.

void init();             // Init the tokenTable, insert reservedWords.

// Emitter, the format is defined by implementation.
void emit(int token, float tokenVal);

// Preprocessing

/* #define <IDENT> <VALUE> */
int defineIndex[TABLESIZE];    // Store IDENTs' location in tokenTable.
char* strIndex[TABLESIZE];     // Store VALUEs as strings. 
char defineBuffer[BUFFERSIZE]; // Place to store VALUES.
int definePtr = 0;             // record the size of index.
int defineBufferPtr = 0;             // record the end of used buffer.

char* readUntilSpace(FILE*);  // Read from target file to preBuffer until meeting spaces.
void readUntilChar(FILE*);    // Skip spaces, tabs and enters between keywords.
void readLine(FILE*);
void copyFile(FILE*, FILE*);

void preProcess(const char*,const char*);

/* If There're include instructions in file, one time of
 * preprocess may not be enough. To set ifClean to 0, we
 * could know that there're still include instructions. */
int ifClean = 1;              

char* tempFileNames[2] = {"temp1", "temp2"};
                                 

// Lexical analysis
int getToken();

// Syntatic Tree

// An attribute might be the name of the operation/period or the token's value.
typedef union {
  char* name;
  float value;
} attribute;

// Use linked list to store childs of a node.
struct _child {
  struct _node* location;
  struct _child* next;
};

struct _node {
  attribute property;
  int type;
  struct _child* head;
};

typedef struct _node node;
typedef struct _node* link;
typedef struct _child child;

link mkNode(int, attribute);
attribute mkNumProp(float);
attribute mkStrProp(char*);
void addChild(link, link);
void indent(int);

void traverse(link, int);

// Grammatical analysis
link parse();
link externalDef();
link funcDef();
link compState();
link statement();
link stat();
link compStateOrStat();
link expr();
link priorN1();
link priorN2();
link prior0();
link prior1();
link prior2();
link prior3();
link prior4();
link factor();
link argSeq();

