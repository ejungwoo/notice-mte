#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NoticeTIAM.h"

int main(int argc, char *argv[])
{
  int sid = 0;                  // temporary use for Notice VME controller
  unsigned long mid;
  int vme_Handle;               
  unsigned long pass;

  if (argc > 1)
    mid = atoi(argv[1]);
  else
    mid = 2;

  // open TIAM
  vme_Handle = TIAM_VMEopen(sid);

  // show present setting
  pass = TIAM_VMEread_ENABLE_PASS(vme_Handle, mid);

  if (pass)
    printf("LTE link PASS is enabled.\n");
  else
    printf("LTE link PASS is disabled.\n");

  printf("Change setting(0 = disable, 1 = enable) : ");
  scanf("%ld", &pass);
  
  // change setting
  TIAM_VMEwrite_ENABLE_PASS(vme_Handle, mid, pass);
  
  // show changed setting
  pass = TIAM_VMEread_ENABLE_PASS(vme_Handle, mid);

  if (pass)
    printf("LTE link PASS is enabled.\n");
  else
    printf("LTE link PASS is disabled.\n");

  // close TIAM
  TIAM_VMEclose(vme_Handle);

  return 0;
}

