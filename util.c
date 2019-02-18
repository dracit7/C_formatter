// 
// util.c
// 
// This file includes some helper functions.
// 


#ifndef _INC_GLOBAL
#include "global.h"
#endif

/* Util functions */

int isDigit(int c) {
  return c<='9' && c>='0';
}

int isAlpha(int c) {
  return (c<='z'&&c>='a')||(c<='Z'&&c>='A');
}

int isConst(int c) {
  return c == INT_CONST || c == FLOAT_CONST || c == CHAR_CONST
      || c == LONG_CONST || c == SHORT_CONST || c == DOUBLE_CONST;
}

int isVar(int c) {
  return c == INT || c == FLOAT || c == CHAR || c == LONG || c == SHORT || c == DOUBLE;
}

// Implementation of string comparasion
// Returns 0 only if two strings are equal.
int strCmp(char* s1,char* s2) {
  int i=0;
  while (*(s1+i)==*(s2+i)) {
    if (*(s1+i) == 0)
      return 0;
    i++;
  }
  return 1;
}

// Copy s2 to s1, returns the index of s2's end in s1
char* strCpy(char* s1,const char* s2) {
  char* r=s1;
  if (!(s1 && s2)) exception("strCpy received null pointer.");
  while ((*r++ = *s2++)!='\0') ;
  return r-1;
}

// Read characters from a file ptr until meeting spaces(or tabs/enters).
// Returns the pointer to the head of the read string.
char* readUntilSpace(FILE* input) {
  char c = fgetc(input);
  int i = 0;
  while (c != ' ' && c != '\n' && c != '\t') {
    preBuffer[i++] = c;
    c = fgetc(input);
  }
  preBuffer[i] = 0;
  return preBuffer;
}

// Clear consecutive spaces from a file ptr.
void readUntilChar(FILE* input) {
  char c;
  while ((c = fgetc(input)) == ' ');
  ungetc(c,input);
}

// Read a line from a file ptr.
void readLine(FILE* input) {
  char c;
  while ((c = fgetc(input)) != '\n') ;
}

// Copy the content of file a to file b
int copyFile(FILE* input, FILE* output) {
  int lineNum = 0;
  char c;
  while ((c = fgetc(input)) != EOF) {
    if (c == '\n') lineNum++; 
    fputc(c,output);
  }
  return lineNum;
}

// Add a file range information to rangeList.
void addRange(char* file, int beg, int end) {
  if (rangeList == 0) {
    rangeList = (range*)malloc(sizeof(range));
    rangeList->fileName = file;
    rangeList->begin = beg;
    rangeList->end = end;
    rangeList->next = 0;
    return;
  }
  range* ptr = rangeList;
  while (ptr->next) ptr = ptr->next;
  ptr->next = (range*)malloc(sizeof(range));
  ptr->next->fileName = file;
  ptr->next->begin = beg;
  ptr->next->end = end;
  ptr->next->next = 0;
}

/* Error Handling */

void error(char* message) {
  range* ptr = rangeList;
  int lineNum = lineSerial;
  while (ptr) {
    if (lineSerial >= ptr->begin && lineSerial <= ptr->end) {
      printf("Error: In file \"%s\":\n",ptr->fileName);
      printf("\tline(%d): %s\n",lineSerial - ptr->begin + 1,message);
      remove(tempFileNames[0]);
      remove(tempFileNames[1]);
      exit(1);
    }
    lineNum -= ptr->end - ptr->begin;
    ptr = ptr->next;
  }
  printf("\nError: line(%d): %s\n",lineNum,message);
  remove(tempFileNames[0]);
  remove(tempFileNames[1]);
  exit(1);
}

void exception(char* message) {
  printf("\nException: %s\n",message);
  remove(tempFileNames[0]);
  remove(tempFileNames[1]);
  exit(1);
}

/* Functions for the tokenTable */

// Find a token with name <str> from the tokenTable.
// If such token exists, return its index;
// elsewise, return -1 instead.
int find(char* str) {
  for (int i=0; i < symPtr; i++) {
    if (strCmp(str,tokenTable[i].content) == 0)
      return i;
  }
  return -1;
}

// Insert a token into the tokenTable.
// Such token has name <str> and tokenValue <token>.
int insert(char* str, int token) {
  if (symPtr >= TABLESIZE) exception("The tokenTable is full.");
  tokenTable[symPtr].content = str;
  tokenTable[symPtr++].type = token;
  return symPtr-1;
}

// Init the tokenTable, insert keywords.
// This step is vital, skipping it would cause a segment fault.
void init() {
  for (token* ptr = reservedWords; ptr->type; ptr++) 
    insert(ptr->content, ptr->type);
}

/* Emitter */

void emit(int token, float tokenVal) {
  switch (token) {
    case IDENT: printf("%s",tokenTable[(int)tokenVal].content);break;
    case INT_CONST: printf("%d",(int)tokenVal);break;
    case FLOAT_CONST: printf("%f",tokenVal);break;
    case CHAR_CONST: printf("\'%c\'",(int)tokenVal);break;
    case PLUS: printf(" + "); break;
    case MINUS: printf(" - "); break;
    case MULTI: printf(" * "); break;
    case DIV: printf(" / "); break;
    case MOD: printf(" %% "); break;
    case EQUAL: printf(" == "); break;
    case ASSIGN: printf(" = "); break;
    case GREATER: printf(" > "); break; 
    case LESS: printf(" < "); break; 
    case GEQUAL: printf(" >= "); break; 
    case LEQUAL: printf(" <= "); break; 
    case NOT: printf(" !"); break; 
    case NEQUAL: printf(" != "); break; 
    case INT: printf("int "); break; 
    case FLOAT: printf("float "); break; 
    case CHAR: printf("char "); break; 
    case VOID: printf("void "); break;
    case IF: printf("if "); break; 
    case ELSE: printf(" else "); break; 
    case WHILE: printf("while "); break; 
    case RETURN: printf("return "); break; 
    case SEMI: printf(";\n"); break; 
    case COMMA: printf(", "); break; 
    case COLON: printf(": "); break; 
    case LPAREN: printf("("); break; 
    case RPAREN: printf(")"); break; 
    case LBRACE: printf(" {\n"); break; 
    case RBRACE: printf("}\n"); break; 
    default: printf("token: %d\n",token);
  }
}

/* Functions for the syntatic tree */

// Make a new node of the syntatic tree.
// Returns the pointer to this node.
link mkNode(int Type, attribute prop) {
  link newNode = (link)malloc(sizeof(node));
  newNode->property = prop;
  newNode->type = Type;
  newNode->head = 0;
  return newNode;
}

// Make an number attribute for the tree node.
attribute mkNumProp(float prop) {
  attribute attr;
  attr.value = prop;
  return attr;
}

// Make an string attribute for the tree node.
attribute mkStrProp(char* prop) {
  attribute attr;
  attr.name = prop;
  return attr;
}

// Add a child node pointed by <childPtr> to a node pointed by <root>
void addChild(link root, link childPtr) {
  child* ptr = root->head;
  while (ptr && ptr->next) ptr = ptr->next; 
  if (ptr) {
    ptr->next = (child*)malloc(sizeof(child));
    ptr->next->location = childPtr;
    ptr->next->next = 0;
  } else {
    ptr = (child*)malloc(sizeof(child));
    ptr->location = childPtr;
    ptr->next = 0;
    root->head = ptr;
  }
}

// Format the output
void indent(int layer) {
  for (int i=0; i<layer; i++) putchar('\t');
}

// Traverse the syntatic tree and print the structure of the program to stdout.
void traverse(link root, int layer) {
  child* ptr;
  switch(root->type) {
    case PROGREM:
      indent(layer);
      printf("Program:\n");
      ptr = root->head;
      while (ptr) {
        traverse(ptr->location,layer+1);
        ptr = ptr->next;
      }
      return;
    case FUNCDEF:
      indent(layer);
      printf("Function Defination:\n");
      indent(layer+1);
      printf("Function Name: %s\n",root->property.name);
      traverse(root->head->location, layer+1);             // ParamList
      traverse(root->head->next->location, layer+1);       // Complicated Statement
      traverse(root->head->next->next->location, layer+1); // Return Type
      return;
    case VARDEF:
      indent(layer);
      printf("Variable Defination:\n");
      indent(layer+1);
      printf("Variable type: ");
      switch (root->head->location->type) {
        case INT: printf("integer\n"); break;
        case FLOAT: printf("float\n"); break;
        case CHAR: printf("character\n"); break;
        case LONG: printf("long number\n"); break;
        case SHORT: printf("short number\n"); break;
        case DOUBLE: printf("double\n"); break;
        default: error("Invalid datatype.");
      }
      ptr = root->head->next;
      while (ptr) {
        traverse(ptr->location, layer+1);
        ptr = ptr->next;
      }
      return;
    case PARAMLIST:
      indent(layer);
      printf("Parameters:");
      ptr = root->head;
      if (ptr == 0) printf(" <NONE>");
      putchar('\n');
      while (ptr) {
        traverse(ptr->location, layer+1);
        ptr = ptr->next;
      }
      return;
    case PARAM:
      switch ((int)(root->property).value) {
        case INT: indent(layer); printf("Type: int\n"); break;
        case FLOAT: indent(layer); printf("Type: float\n"); break;
        case CHAR: indent(layer); printf("Type: char\n"); break;
        case LONG: indent(layer); printf("Type: long\n"); break;
        case SHORT: indent(layer); printf("Type: short\n"); break;
        case DOUBLE: indent(layer); printf("Type: double\n"); break;
        default: error("Parameters can not have type void.");
      }
      traverse(root->head->location, layer);
      return;
    case COMPSTATE:
      ptr = root->head;
      while (ptr) {
        traverse(ptr->location, layer);
        ptr = ptr->next;
      }
      return;
    case RETURNTYPE:
      switch ((int)(root->property).value) {
        case INT: indent(layer); printf("Return value's type: int\n"); break;
        case FLOAT: indent(layer); printf("Return value's type: float\n"); break;
        case CHAR: indent(layer); printf("Return value's type: char\n"); break;
        case LONG: indent(layer); printf("Return value's type: long\n"); break;
        case SHORT: indent(layer); printf("Return value's type: short\n"); break;
        case DOUBLE: indent(layer); printf("Return value's type: double\n"); break;
        case VOID: indent(layer); printf("No return value.\n");break;
        default: exception("Return type is abnormal.");
      }
      return;
    case INT_CONST:
      indent(layer);
      printf("Int constant: %d\n", (int)(root->property.value));
      return;
    case FLOAT_CONST:
      indent(layer);
      printf("Float constant: %f\n", root->property.value);
      return;
    case CHAR_CONST:
      indent(layer);
      printf("Char constant: \'%c\'\n", (int)(root->property.value));
      return;
    case LONG_CONST:
      indent(layer);
      printf("Long constant: %ld\n", (long)(root->property.value));
      return;
    case SHORT_CONST:
      indent(layer);
      printf("Short constant: %d\n", (short)(root->property.value));
      return;
    case DOUBLE_CONST:
      indent(layer);
      printf("Double constant: %f\n", (double)(root->property.value));
      return;
    case INT: case CHAR: case FLOAT: case VOID:
    case LONG: case SHORT: case DOUBLE:
      switch (root->type) {
        case INT: indent(layer); printf("Type: int\tValue: %d\n",(int)(root->property.value)); break;
        case FLOAT: indent(layer); printf("Type: float\tValue: %f\n",root->property.value); break;
        case CHAR: indent(layer); printf("Type: char\tValue: %c\n",(int)(root->property.value)); break;
        case LONG: indent(layer); printf("Type: long\tValue: %ld\n",(long)(root->property.value)); break;
        case SHORT: indent(layer); printf("Type: short\tValue: %d\n",(short)(root->property.value)); break;
        case DOUBLE: indent(layer); printf("Type: double\tValue: %f\n",(double)(root->property.value)); break;
        default: error("Variables can not have type void.");
      }
      return;
    case VAR:
      indent(layer);
      printf("Identifier: %s\n",root->property.name);
      if (root->head) {
        indent(layer+1);
        printf("Initial Value:\n");
        traverse(root->head->location, layer+2);
      }
      return;
    case VARSEQ:
      indent(layer);
      printf("Array: %s\n",root->property.name);
      if (root->head) {
        indent(layer+1);
        printf("Size:\n");
        traverse(root->head->location, layer+2);
      } else error("Array declaration without size.");
      if (root->head->next) {
        indent(layer+1);
        printf("Initial Value:\n");
        traverse(root->head->next->location, layer+2);
      }
      break;
    case RETURN:
      indent(layer); 
      printf("RETURN statement: \n");
      if (root->head->location == 0) {
        indent(layer+1);
        printf("<NONE>\n");
      } else traverse(root->head->location, layer+1);
      return;
    case IF:
      if (strCmp(root->property.name,"IF") == 0) {
        indent(layer); 
        printf("IF expression:\n");
        indent(layer+1); 
        printf("Condition:\n");
        traverse(root->head->location, layer+2);
        indent(layer+1); 
        printf("If condition is true:\n");
        traverse(root->head->next->location, layer+2);
      } else if (strCmp(root->property.name,"IF_ELSE") == 0) {
        indent(layer); 
        printf("IF_ELSE expression:\n");
        indent(layer+1); 
        printf("Condition:\n");
        traverse(root->head->location, layer+2);
        indent(layer+1); 
        printf("If condition is true:\n");
        traverse(root->head->next->location, layer+2);
        indent(layer+1); 
        printf("Else:\n");
        traverse(root->head->next->next->location, layer+2);
      } else exception("Exists IF expr in other forms.");
      return;
    case WHILE:
      indent(layer);
      printf("WHILE expression:\n");
      indent(layer+1);
      printf("Condition:\n");
      if (root->head->location == 0)
        error("Error: WHILE expression without condition.");
      traverse(root->head->location, layer+2);
      indent(layer+1);
      printf("While condition is true:\n");
      if (root->head->next->location == 0 || root->head->next->location->head == 0) {
        indent(layer+2);
        printf("<NONE>\n");
      } else traverse(root->head->next->location, layer+2);
      return;
    case FOR:
      indent(layer);
      printf("FOR expression:\n");
      indent(layer+1);
      printf("Initial statement:\n");
      if (root->head->location == 0 || root->head->location->head == 0) {
        indent(layer+2);
        printf("<NONE>\n");
      } else traverse(root->head->location, layer+2);
      indent(layer+1);
      printf("Condition:\n");
      if (root->head->next->location == 0)
        error("Error: FOR expression without condition.");
      traverse(root->head->next->location,layer+2);
      indent(layer+1);
      printf("Circulator:\n");
      if (root->head->next->next->location == 0 || root->head->next->next->location->head == 0) {
        indent(layer+2);
        printf("<NONE>\n");
      } else traverse(root->head->next->next->location, layer+2);
      indent(layer+1);
      printf("While condition is true:\n");
      if (root->head->next->next->next->location == 0 || root->head->next->next->next->location->head == 0) {
        indent(layer+2);
        printf("<NONE>\n");
      } else traverse(root->head->next->next->next->location, layer+2);
      break;
    case BREAK:
      indent(layer);
      printf("Break statement\n");
      break;
    case CONTINUE:
      indent(layer);
      printf("Continue statement\n");
      break;
    case ASSIGN:
      indent(layer);
      printf("Operation: Assign(=)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case AND:
      indent(layer);
      printf("Operation: And(&&)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case OR:
      indent(layer);
      printf("Operation: Or(||)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case EQUAL:
      indent(layer);
      printf("Operation: Equal(==)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case NEQUAL:
      indent(layer);
      printf("Operation: Not equal(!=)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case GREATER:
      indent(layer);
      printf("Operation: IfGreater(>)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case LESS:
      indent(layer);
      printf("Operation: IfLess(<)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case GEQUAL:
      indent(layer);
      printf("Operation: Greater or equal(>=)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case LEQUAL:
      indent(layer);
      printf("Operation: Less or equal(<=)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case PLUS:
      indent(layer);
      printf("Operation: Plus(+)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case MINUS:
      indent(layer);
      printf("Operation: Minus(-)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case MULTI:
      indent(layer);
      printf("Operation: Multiply(*)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case DIV:
      indent(layer);
      printf("Operation: Division(/)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case MOD:
      indent(layer);
      printf("Operation: Modulus(%%)\n");
      indent(layer+1);
      printf("Operands:\n");
      traverse(root->head->location, layer+2);
      traverse(root->head->next->location, layer+2);
      return;
    case NOT:
      indent(layer);
      printf("Operation: Not(!)\n");
      indent(layer+1);
      printf("Operand:\n");
      traverse(root->head->location, layer+2);
      return;
    case FUNC:
      indent(layer);
      printf("Function call:\n");
      indent(layer+1);
      printf("Function name: %s\n", (root->property).name);
      traverse(root->head->location, layer+1);
      return;
    case ARGSEQ:
      indent(layer);
      printf("Arguments:\n");
      ptr = root->head;
      if (ptr->location == 0) {
        indent(layer+1);
        printf("<NONE>\n");
      } else while (ptr) {
        traverse(ptr->location, layer+1);
        ptr = ptr->next;
      }
      return;
  }
}

// Traverse the syntatic tree and print the formatted program
// to a new file named <formatOutput>.
void format(link root, int layer) {
  child* ptr;
  switch(root->type) {
    case PROGREM:
      ptr = root->head;
      while (ptr) {
        format(ptr->location,layer);
        ptr = ptr->next;
        printf("\n");
      }
      return;
    case FUNCDEF:
      printf("\n");
      indent(layer);
      format(root->head->next->next->location, layer); // Print return type
      printf("%s",root->property.name);                        // Print function name
      printf("(");
      format(root->head->location, layer);             // Print paramList
      printf(") ");
      format(root->head->next->location, layer);       // Print complicated statement
      return;
    case VARDEF:
      // Variable Type
      switch (root->head->location->type) {
        case INT: printf("int "); break;
        case FLOAT: printf("float "); break;
        case CHAR: printf("char "); break;
        case LONG: printf("long "); break;
        case SHORT: printf("short "); break;
        case DOUBLE: printf("double "); break;
        default: exception("Invalid datatype.");
      }
      ptr = root->head->next;
      // Variables
      if (ptr) {
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      while (ptr) {
        printf(", ");
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      printf("; ");
      return;
    case PARAMLIST:
      ptr = root->head;
      if (ptr) {
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      while (ptr) {
        printf(", ");
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      return;
    case PARAM:
      switch ((int)(root->property).value) {
        case INT: printf("int "); break;
        case FLOAT: printf("float "); break;
        case CHAR: printf("char "); break;
        case LONG: printf("long "); break;
        case SHORT: printf("short "); break;
        case DOUBLE: printf("double "); break;
        default: exception("Parameters can not have type void.");
      }
      format(root->head->location, layer);
      return;
    case COMPSTATE:
      ptr = root->head;
      printf("{\n");
      while (ptr) {
        indent(layer+1);
        format(ptr->location, layer+1);
        ptr = ptr->next;
        printf("\n");
      }
      indent(layer);
      printf("}");
      return;
    case RETURNTYPE:
      switch ((int)(root->property).value) {
        case INT: printf("int "); break;
        case FLOAT: printf("float "); break;
        case CHAR: printf("char "); break;
        case VOID: printf("void ");break;
        case LONG: printf("long "); break;
        case SHORT: printf("short "); break;
        case DOUBLE: printf("double "); break;
        default: exception("Return type is abnormal.");
      }
      return;
    case INT_CONST:
      printf("%d", (int)(root->property.value));
      return;
    case FLOAT_CONST:
      printf("%f", root->property.value);
      return;
    case CHAR_CONST:
      printf("\'%c\'", (int)(root->property.value));
      return;
    case LONG_CONST:
      printf("%ld", (long)(root->property.value));
      return;
    case SHORT_CONST:
      printf("%d", (short)(root->property.value));
      return;
    case DOUBLE_CONST:
      printf("%f", (double)(root->property.value));
      return;
    case INT: case CHAR: case FLOAT: case VOID:
    case LONG: case SHORT: case DOUBLE:
      switch (root->type) {
        case INT: printf("%d",(int)(root->property.value)); break;
        case FLOAT: printf("%f",root->property.value); break;
        case CHAR: printf("\'%c\'",(int)(root->property.value)); break;
        case LONG: printf("%ld",(long)(root->property.value)); break;
        case SHORT: printf("%d",(short)(root->property.value)); break;
        case DOUBLE: printf("%f",(double)(root->property.value)); break;
        default: exception("Variables can not have type void.");
      }
      return;
    case VAR:
      printf("%s",root->property.name);
      if (root->head) {
        printf(" = ");
        format(root->head->location, layer+1);
      }
      return;
    case VARSEQ:
      printf("%s",root->property.name);
      if (root->head) {
        printf("[");
        format(root->head->location, layer);
        printf("]");
      } else error("Array declaration without size.");
      if (root->head->next) {
        printf(" = ");
        format(root->head->next->location, layer);
      }
      return;
    case RETURN:
      printf("return ");
      if (root->head->location != 0) format(root->head->location, layer);
      printf(";");
      return;
    case IF:
      if (strCmp(root->property.name,"IF") == 0) {
        printf("if (");
        format(root->head->location, layer);
        printf(") ");
        if (root->head->next->location == 0 || root->head->next->location->head == 0) {
          printf(";");
        } else format(root->head->next->location, layer);
      } else if (strCmp(root->property.name,"IF_ELSE") == 0) {
        printf("if (");
        format(root->head->location, layer);
        printf(") ");
        if (root->head->next->location == 0 || root->head->next->location->head == 0) {
          printf(";\n");
        } else format(root->head->next->location, layer);
        printf("else ");
        if (root->head->next->next->location == 0 || root->head->next->next->location->head == 0) {
          printf(";");
        } else format(root->head->next->next->location, layer);
      } else exception("Exists IF expr in other forms.");
      return;
    case WHILE:
      printf("while (");
      if (root->head->location == 0)
        error("Error: WHILE expression without condition.");
      format(root->head->location, layer);
      printf(") ");
      if (root->head->next->location == 0 || root->head->next->location->head == 0) {
        printf(";");
      } else format(root->head->next->location, layer);
      return;
    case FOR:
      printf("for (");
      if (root->head->location == 0 || root->head->location->head == 0) ;
      else format(root->head->location, layer);
      if (root->head->next->location == 0)
        exception("FOR expression without condition.");
      format(root->head->next->location,layer);
      printf("; ");
      if (root->head->next->next->location == 0 || root->head->next->next->location->head == 0) ;
      else format(root->head->next->next->location, layer);
      printf(") ");
      if (root->head->next->next->next->location == 0 || root->head->next->next->next->location->head == 0) {
        printf(";");
      } else format(root->head->next->next->next->location, layer);
      break;
    case BREAK:
      printf("break;");
      break;
    case CONTINUE:
      printf("continue;");
      break;
    case ASSIGN:
      format(root->head->location, layer);
      printf(" = ");
      format(root->head->next->location, layer);
      return;
    case AND:
      format(root->head->location, layer);
      printf(" && ");
      format(root->head->next->location, layer);
      return;
    case OR:
      format(root->head->location, layer);
      printf(" || ");
      format(root->head->next->location, layer);
      return;
    case EQUAL:
      format(root->head->location, layer);
      printf(" == ");
      format(root->head->next->location, layer);
      return;
    case NEQUAL:
      format(root->head->location, layer);
      printf(" != ");
      format(root->head->next->location, layer);
      return;
    case GREATER:
      format(root->head->location, layer);
      printf(" > ");
      format(root->head->next->location, layer);
      return;
    case LESS:
      format(root->head->location, layer);
      printf(" < ");
      format(root->head->next->location, layer);
      return;
    case GEQUAL:
      format(root->head->location, layer);
      printf(" >= ");
      format(root->head->next->location, layer);
      return;
    case LEQUAL:
      format(root->head->location, layer);
      printf(" <= ");
      format(root->head->next->location, layer);
      return;
    case PLUS:
      format(root->head->location, layer);
      printf(" + ");
      format(root->head->next->location, layer);
      return;
    case MINUS:
      format(root->head->location, layer);
      printf(" - ");
      format(root->head->next->location, layer);
      return;
    case MULTI:
      format(root->head->location, layer);
      printf(" * ");
      format(root->head->next->location, layer);
      return;
    case DIV:
      format(root->head->location, layer);
      printf(" / ");
      format(root->head->next->location, layer);
      return;
    case MOD:
      format(root->head->location, layer);
      printf(" %% ");
      format(root->head->next->location, layer);
      return;
    case NOT:
      printf("!");
      format(root->head->location, layer);
      return;
    case FUNC:
      printf("%s(", (root->property).name);
      format(root->head->location, layer);
      printf(");");
      return;
    case ARGSEQ:
      ptr = root->head;
      if (ptr->location != 0) {
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      while (ptr) {
        printf(", ");
        format(ptr->location, layer);
        ptr = ptr->next;
      }
      return;
  }
}
