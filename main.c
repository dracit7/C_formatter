
#include "global.h"
#include "util.c"
#include "preprocess.c"
#include "parser.c"

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
  symPtr = 0; // Clear the tokenTable
  init();
  filePtr = fopen(tempFileNames[i],"r");
  if (!filePtr) exception("Cannot open target file.");
  link root = parse();
  // Traverse the syntatic tree.
  traverse(root,0);
  freopen(formatOutput,"w",stdout);
  format(root,0);
  // Clean the environment.
  fclose(filePtr);
  remove(tempFileNames[0]);
  remove(tempFileNames[1]);
  return 0;
}