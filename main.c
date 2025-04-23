#include <stdio.h>
#define _IO volatile 
int g_var = 1234;

int main(void)
{
  for(_IO int i=0; i<20;i++)
  {
    g_var++; 
  }
  return 0;
}