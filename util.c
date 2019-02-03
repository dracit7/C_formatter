
#ifndef _INC_GLOBAL
#include "global.h"
#endif

// Util functions
int isDigit(int c) {
  return c<='9' && c>='0';
}

int isAlpha(int c) {
  return (c<='z'&&c>='a')||(c<='Z'&&c>='A');
}

int strCmp(char* s1,char* s2) {
  int i=0;
  while (*(s1+i)==*(s2+i)) {
    if (*(s1+i) == 0)
      return 0;
    i++;
  }
  return 1;
}

char* strCpy(char* s1,const char* s2) {
  char* r=s1;
  if (!(s1 && s2)) exception("strCpy received null pointer.");
  while ((*r++ = *s2++)!='\0') ;
  return r-1;
}

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

void readUntilChar(FILE* input) {
  char c;
  while ((c = fgetc(input)) == ' ');
  ungetc(c,input);
}

void readLine(FILE* input) {
  char c;
  while ((c = fgetc(input)) != '\n') ;
}

void copyFile(FILE* input, FILE* output) {
  char c;
  while ((c = fgetc(input)) != EOF) fputc(c,output);
}

// Error Handling
void error(char* message) {
  printf("\nError: line(%d): %s\n",lineSerial,message);
  exit(1);
}

void exception(char* message) {
  printf("\nException: %s\n",message);
  exit(1);
}

// Functions for the tokenTable
int find(char* str) {
  for (int i=0; i < symPtr; i++) {
    if (strCmp(str,tokenTable[i].content) == 0)
      return i;
  }
  return -1;
}

int insert(char* str, int token) {
  if (symPtr >= TABLESIZE) exception("The tokenTable is full.");
  tokenTable[symPtr].content = str;
  tokenTable[symPtr++].type = token;
  return symPtr-1;
}

void init() {
  for (token* ptr = reservedWords; ptr->type; ptr++) 
    insert(ptr->content, ptr->type);
}

// Emitter
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

// Syntatic Tree

link mkNode(int Type, attribute prop) {
  link newNode = (link)malloc(sizeof(node));
  newNode->property = prop;
  newNode->type = Type;
  newNode->head = 0;
  return newNode;
}

attribute mkNumProp(float prop) {
  attribute attr;
  attr.value = prop;
  return attr;
}

attribute mkStrProp(char* prop) {
  attribute attr;
  attr.name = prop;
  return attr;
}

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

void indent(int layer) {
  for (int i=0; i<layer; i++) putchar('\t');
}

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
    case INT: case CHAR: case FLOAT: case VOID:
      switch (root->type) {
        case INT: indent(layer); printf("Type: int\tValue: %d\n",(int)(root->property.value)); break;
        case FLOAT: indent(layer); printf("Type: float\tValue: %f\n",root->property.value); break;
        case CHAR: indent(layer); printf("Type: char\tValue: %c\n",(int)(root->property.value)); break;
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
