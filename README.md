hashtable_cpu

CPU version 
how to compile:
gcc -o hashtable_cpu hashtable_cpu.c -lrt
usage:

In main function  you can chage the setting
  
 a. increasing  words in char *s[]
         char *s[]={"steve","bOB","apple","ban","Johnson", 
        "banana","ice","happy","home","hello","love","wen","danny"
        ,"dog","hot" ,"cold","fato","fatrabbit","jerry","tux"};
		
  int wordsToHash = 20;//number of words
 
 b.   char *find[]={"hello","home"};
    listShow = lookup_string(my_hash_table,find[1]);
	
	change the word you want to try(now only one word)
 
 c. may change bucket number 
  int size_of_table = 12;//num of bucket in hashtable  



OMP version 

compile: gcc -o hashtable_omp hashtable_omp.c -fopenmp -lrt
usage: ./hashtable_omp <thread NUM>

pth version
compile: gcc -g -Wall -o hashtable_pth hashtable_pth.c -pthread -lrt
usage: ./hashtable_pth <thread NUM># pthread_Hash_C
