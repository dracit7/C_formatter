// 
// global.h
// 
// The header file of the formatter, includes global declarations.
// 


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
  INT_CONST, FLOAT_CONST, CHAR_CONST, LONG_CONST, SHORT_CONST, DOUBLE_CONST, // Const types
  PLUS, MINUS, MULTI, DIV, MOD, EQUAL, ASSIGN, INCRE, DECRE, // Arithmetic operators
  GREATER, LESS, GEQUAL, LEQUAL, NOT, NEQUAL, AND, OR,  // Comparitive operators
  INT, FLOAT, CHAR, VOID, LONG, SHORT, DOUBLE, // Variable types
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
#define KEYWORD_NUM 14   // Number of keywords in tokenTable.
#define NONE -2

// Global variables
float tokenValue = NONE; // The value of the Token
int lineSerial = 1;
char buffer[BUFFERSIZE]; // Store all identifiers' names linearly in a buffer.
int bufferPtr = 0;       // Point to the next byte of the end of the buffer's used space.
FILE *filePtr;           // Input file pointer
int tokenPtr = 0;        // This is the pointer for grammatical analysis.
char preBuffer[BUFFERSIZE]; // Buffer for preprocession.
char formatOutput[] = "formatted.c";

// Util Functions
int isDigit(int); // Judge if a character represents a number
int isAlpha(int); // Judge if a character represents a letter 
int isConst(int); // Judge if a token represents a constant
int strCmp(char*, char*); // Enhanced strcmp, able to compare string variables.
char* strCpy(char*, const char*); // Enhanced strcpy, returns the end of s1

// Error Handling
void error(char*);      // Report an error and the line serial at present, then exit
void exception(char*);  // Throw an exception and exit

// SymbolTable
int symPtr = 0;  // The present size of symbolTable

typedef struct {
  char* content; // The name of identifier or keyword token
  int type;      // The type of token (IDENT or keyword types)
} token;  // token structures in symbolTable

token tokenTable[TABLESIZE]; // Body of the symbolTable

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
  "long", LONG,
  "short", SHORT,
  "double", DOUBLE,
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
int defineBufferPtr = 0;       // record the end of used buffer.
char filenameBuffer[BUFFERSIZE]; // record filenames.
int filenamePtr = 0;

struct _range {
  char* fileName;
  int begin;
  int end;
  struct _range* next; 
};  // Record the range of a file in preprocessed sourcefile.

typedef struct _range range;
range* rangeList = 0;

void addRange(char*, int, int);

char* readUntilSpace(FILE*);  // Read from target file to preBuffer until meeting spaces.
void readUntilChar(FILE*);    // Skip spaces, tabs and enters between keywords.
void readLine(FILE*);         // Skip the present line.
int copyFile(FILE*, FILE*);   // Copy the content of File A to File B, returning lineNum of File A.

void preProcess(const char*,const char*);

/* If There're include instructions in file, one time of
 * preprocess may not be enough. To set ifClean to 0, we
 * could know that there're still include instructions. */
int ifClean = 1;              

/* Preprocess would generate temp files from the input file.
 * Until there's no include instructions in temp file, preprocess() 
 * would be called again and again. */
char* tempFileNames[2] = {"temp1", "temp2"};
                                 

// Lexical analysis
int getToken();  // Get a token from input.

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

link mkNode(int, attribute);    // Make a node with its nodeType and attribute.
attribute mkNumProp(float);     // Wrap a float number into an attribute.
attribute mkStrProp(char*);     // Wrap a string into an attribute.
void addChild(link, link);      // Add node B to node A's childlist.
void indent(int);               // Formatter, make indent

void traverse(link, int);
void format(link, int);

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

