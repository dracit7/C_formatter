
#include "global.h"
#include "preprocess.c"
#include "parser.c"
#include "util.c"

int main(int argc, char const *argv[])
{
  if (argc != 2) exception("Argument incorrect.");
  // Insert keywords to the symbolTable.
  init();
  // Preprocess period.
  int i = 0;
  preProcess(argv[1],tempFileNames[0]);
  while (!ifClean) {
    ifClean = 1;
    preProcess(tempFileNames[i],tempFileNames[!i]);
    i = !i;
  }
  // Parse period.
  printf("\nComments:\n\n");
  symPtr = 0; // Clear the tokenTable
  // Init the token Table
  init();
  filePtr = fopen(tempFileNames[i],"r");
  if (!filePtr) exception("Cannot open target file.");
  link root = parse();
  // Traverse the syntatic tree.
  putchar('\n');
  traverse(root,0);
  freopen(formatOutput,"w",stdout);
  format(root,0);
  // Clean the environment.
  fclose(filePtr);
  remove(tempFileNames[0]);
  remove(tempFileNames[1]);
  return 0;
}