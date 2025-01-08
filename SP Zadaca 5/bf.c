#include <stdio.h>
#include <stdlib.h>
#define ARRAYSIZE 65536
#define MAXCODESIZE 65536
int stack[MAXCODESIZE], stackp; //za neusaglasene zagrade.
char code[MAXCODESIZE]; int codep, codelength; //program 
short int array[ARRAYSIZE], memp; //celije
int targets[MAXCODESIZE]; //pozicije zagrada
int c;
FILE *prog;
int main(int argc, char **argv){
  if (argc != 2) 
    fprintf(stderr, "Upotreba: bf prog.bf.\n"), exit(1);
  if(!(prog = fopen(argv[1], "r")))          
    fprintf(stderr,"Nisam otvorio %s.\n", argv[1]),exit(1);
    codelength = fread(code, 1, MAXCODESIZE, prog);
    fclose(prog);
    for(codep=0; codep<codelength; codep++){
        if (code[codep]=='[') 
           stack[stackp++]=codep;//stavi '[' na stek
        if (code[codep]==']'){ //Ako je nadjen ']',
            if(stackp==0){ //a nema '[' to je greska
              fprintf(stderr,"Neuparena ']' na %d.", codep);
              exit(1);
            } else {
                --stackp; //pokupi '[' sa vrha steka
                targets[codep]=stack[stackp];   
                targets[stack[stackp]]=codep; 
            }
        }
    }
    if(stackp>0){ 
      fprintf(stderr,"Neuparena '[' na %d.", stack[--stackp]);
      exit(1);
    }
    for(codep=0;codep<codelength;codep++){//Izvrsavanje
         switch(code[codep]){
            case '+': array[memp]++; break;
            case '-': array[memp]--; break;
            case '<': memp--; break;
            case '>': memp++; break;
            case ',': if((c=getchar())!=EOF)  array[memp]=c=='\n'?10:c; 
                      break;
            case '.':  putchar(array[memp]==10?'\n':array[memp]);  break;
            case '[': if(!array[memp])  codep=targets[codep]; break;
            case ']': if(array[memp])   codep=targets[codep]; break;
        }
    }
    exit(0);
}
