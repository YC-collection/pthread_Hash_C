#include <stdio.h>
#include <time.h>
#include <pthread.h>

//compile: gcc -g -Wall -o hashtable_pth hashtable_pth.c -pthread -lrt
/*-----------------Global variable---------------------------*/
typedef struct _list_t_ {
    char *str;
    struct _list_t_ *next;
} list_t;

typedef struct _hash_table_t_ {
    int size;       /* the size of the table */
    list_t **table; /* the table elements */
} hash_table_t;

struct thread_data {
       hash_table_t *my_hash_table;
       char *str; 
};


long thread_count;
pthread_rwlock_t rwlock;
int size_of_table = 12;
char *find[]={"hello","cold","tux","wen"};

int findCnt =4;  //how many  words in *find[]    
int wordsToHash = 20;//Remember to define your size!!!(how many phrase  is s[] )!
int lookUpCnt = 0;//how many word find in hash table
static char *s[]={"steve","bOB","apple","ban","Johnson", 
        "banana","ice","happy","home","hello","love","wen","danny"
        ,"dog","hot" ,"cold","fato","fatrabbit","jerry","tux"};

const int MAX_THREADS = 1024;




/*-----------------Function prototype---------------------------*/
static double diff_in_second(struct timespec t1, struct timespec t2);
hash_table_t *create_hash_table(int size);
unsigned int hash(hash_table_t *hashtable, char *str);
list_t *lookup_string(hash_table_t *hashtable, char *str);
int add_string(hash_table_t *hashtable, char *str);
void free_table(hash_table_t *hashtable);
//int printList( struct  list_t *head_p ); 
void* thread_oparation(void* rank);

/* Only executed by main thread */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
/*-----------------------------------------------------------------*/





/*Nano second Timer*/
static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
/*Hash table Function*/
hash_table_t *create_hash_table(int size)
{
    int i;
    hash_table_t *new_table;
    
    if (size<1) return NULL; /* invalid size for table */

    /* Attempt to allocate memory for the table structure */
    if ((new_table = malloc(sizeof(hash_table_t))) == NULL) {
        return NULL;
    }
    
    /* Attempt to allocate memory for the table itself */
    if ((new_table->table = malloc(sizeof(list_t *) * size)) == NULL) {
        return NULL;
    }

    /* Initialize the elements of the table */
    for( i=0; i<size; i++) new_table->table[i] = NULL;

    /* Set the table's size */
    new_table->size = size;

    return new_table;
}

/*hash function*/
unsigned int hash(hash_table_t *hashtable, char *str)
{
    unsigned int hashval;
    
    /* we start our hash out at 0 */
    hashval = 0;

    /* for each character, we multiply the old hash by 31 and add the current
     * character.  Remember that shifting a number left is equivalent to 
     * multiplying it by 2 raised to the number of places shifted.  So we 
     * are in effect multiplying hashval by 32 and then subtracting hashval.  
     * Why do we do this?  Because shifting and subtraction are much more 
     * efficient operations than multiplication.
     */
    for(; *str != '\0'; str++) hashval = *str + (hashval << 5) - hashval;

    /* we then return the hash value mod the hashtable size so that it will
     * fit into the necessary range
     */
    return hashval % hashtable->size;
}

/*string look up*/
list_t *lookup_string(hash_table_t *hashtable, char *str)
{
    list_t *list;
    unsigned int hashval = hash(hashtable, str);

    /* Go to the correct list based on the hash value and see if str is
     * in the list.  If it is, return return a pointer to the list element.
     * If it isn't, the item isn't in the table, so return NULL.
     */
    for(list = hashtable->table[hashval]; list != NULL; list = list->next) {
        if (strcmp(str, list->str) == 0) return list;
    }
    return NULL;
}

/*add string*/
int add_string(hash_table_t *hashtable, char *str)
{
    list_t *new_list;
    list_t *current_list;
    unsigned int hashval = hash(hashtable, str);

    /* Attempt to allocate memory for list */
    if ((new_list = malloc(sizeof(list_t))) == NULL) return 1;

    /* Does item already exist? */
    current_list = lookup_string(hashtable, str);
        /* item already exists, don't insert it again. */
    if (current_list != NULL) return 2;
    /* Insert into list */
    new_list->str = strdup(str);
    new_list->next = hashtable->table[hashval];
    hashtable->table[hashval] = new_list;

    return 0;
}

/*free table*/
void free_table(hash_table_t *hashtable)
{
    int i;
    list_t *list, *temp;

    if (hashtable==NULL) return;

    /* Free the memory for every item in the table, including the 
     * strings themselves.
     */
    for(i=0; i<hashtable->size; i++) {
        list = hashtable->table[i];
        while(list!=NULL) {
            temp = list;
            list = list->next;
            free(temp->str);
            free(temp);
        }
    }

    /* Free the table itself */
    free(hashtable->table);
    free(hashtable);
}

/*printList */
int printList( struct  list_t *head_p ) 
{
    list_t *curr_p = head_p;
     
    while(curr_p != NULL)
    {
        printf("%s ",curr_p->str);
        curr_p = curr_p->next;
    }
    printf("\n");
}   /*printList */
/*------------------------------------------------------------------
 * Function:       thread_oparation 
 * Purpose:        Compleetea the link list oparations by the thread running this 
 * In arg:         rank
 * Ret val:        ignored
 * Globals in:     n, thread_count, mMember, mInsert, mDelete
 * Global in/out:  count_member, count_insert, count_delete 
 */
void* thread_oparation(void* rank ) 
{
    long my_rank = (long) rank;
    int  my_lookupCnt = 0;
    list_t*  listShow= NULL;
    long long i;
    long long my_search = findCnt/thread_count;
    long long my_first_i = my_search*my_rank;
    long long my_last_i = my_first_i + my_search;



    for (i = my_first_i; i < my_last_i; i++) 
    {
       //    pthread_rwlock_wrlock(&rwlock);
       //     listShow = lookup_string( ((struct thread_data*) rank) ->my_hash_table,find[i]);
        //  unsigned int hashval = hash(  ( (struct thread_data*) rank) -> my_hash_table, ( (struct thread_data*) rank)->str);
            list_t *list;
           unsigned int hashval = hash(  ( (struct thread_data*) rank) -> my_hash_table, find[i] );

            /* Go to the correct list based on the hash value and see if str is
             * in the list.  If it is, return return a pointer to the list element.
             * If it isn't, the item isn't in the table, so return NULL.
             */
            for(list = ( ( (struct thread_data*) rank) -> my_hash_table )->table[hashval]; list != NULL; list = list->next) {
                if (strcmp(  ( (struct thread_data*) rank) ->str, list->str) == 0)
                {
                    my_lookupCnt++;
                    printf("\n");
                    return list;   
                }
                 
            }
           

        //    pthread_rwlock_unlock(&rwlock);    
    }
    
    lookUpCnt=lookUpCnt +my_lookupCnt;
    
   return NULL;
}  /* Thread_sum */

/*------------------------------------------------------------------
 * Function:    Get_args
 * Purpose:     Get the command line args
 * In args:     argc, argv
 * Globals out: 
 */
void Get_args(int argc, char* argv[]) {
    if (argc != 2)
    {
        Usage(argv[0]);
    }
    thread_count = strtol(argv[1], NULL, 10);  
    if (thread_count <= 0 || thread_count > MAX_THREADS)
    {
        Usage(argv[0]);
    }
    
    
}  /* Get_args */
/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads> \n", prog_name);
   fprintf(stderr,"Compile using : gcc -o hashtable_pth hashtable_pth.c -pthread -lrt \n");
 
              
   exit(0);
}  /* Usage */

  
void main(int argc, char* argv[])
{
        int i;
        long       thread;  /* Use long in case of a 64-bit system */
        pthread_t* thread_handles;
        
      //  list_t*  listShow= NULL;
        

        /* read command line arguments */
        Get_args(argc, argv); 
        printf("threadNum %ld\n",thread_count);
        //timer
        clock_t         start, stop;
        struct timespec go, end;
        double cpu_time1;

       hash_table_t *my_hash_table;  
        my_hash_table = create_hash_table(size_of_table); 

        struct thread_data* thread_array = malloc(thread_count * sizeof(struct thread_data));
        thread_handles = (pthread_t*) malloc (thread_count*sizeof(pthread_t));

 
  
        clock_gettime(CLOCK_REALTIME, &go);
        start = clock();

        /*initialize*/             
        for( i=0; i<wordsToHash ;i++){
        add_string(my_hash_table,s[i]);
        }

        for (thread = 0; thread < thread_count; thread++)  
        {
           // ( &thread_array[thread] ) -> str = 
         //   strncpy(( &thread_array[thread] ) ->str,find[]);
         //   ( &thread_array[thread] ) -> hashtable = my_hash_table;

            pthread_create(&thread_handles[thread], NULL,thread_oparation , (void*) &thread_array[thread]);  
        }
         
        for (thread = 0; thread < thread_count; thread++) 
        {
            pthread_join(thread_handles[thread], NULL); 
        }
         
        pthread_rwlock_destroy(&rwlock);


//        listShow = lookup_string(my_hash_table,find[1]);

        stop = clock();
        clock_gettime(CLOCK_REALTIME, &end);
        cpu_time1 = diff_in_second(go, end);
        float   elapsedTime = (float)(stop - start) /(float)CLOCKS_PER_SEC * 1000.0f;
        printf( "Time to hash:  %3.1f ms\n", elapsedTime );
        printf("execution time of cpu : %lf sec\n", cpu_time1);
        printf("Found the word list \n");
  //      printList(listShow);
        printf("-------------------------- \n");  



        printf("Print hash table content \n");
        for(i=0; i<size_of_table ;i++)
        {
         printList(my_hash_table->table[i]);
        printf("\n");
        }

        free_table(my_hash_table);
}

