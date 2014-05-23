#include <stdio.h>
#include <string.h>

int do_string(char *string)
{
 int i, length;
 i = 0;
 length = strlen(string);

 while(i<length)
 {
  switch(string[i])
  {
   case '\\':
        i++;
        if(i == length)
         return 1;

        switch(string[i])
        {
         case 'n':
              printf("\n");
              break;

         case '\\':
              printf("\\");
              break;

         case '"':
              printf("\"");
              break;

         default:
              printf("--bad escape code '\\%c'--", string[i]);
              return 1;
        }
        break;

   default:
        printf("%c", string[i]);
  }
  i++;
 }

 return 0; 
}

int main(int argc, char **argv)
{
 int x, newline;

 newline = 1;

 for(x=1; x<argc; x++)
 {
  if(do_string(argv[x]))
   return 1;
 }

 if(newline)
  printf("\n");

 return 0;
}

