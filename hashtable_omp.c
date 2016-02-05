#include <stdio.h>
#include <time.h>


//compile: gcc -o hashtable_omp hashtable_omp.c -fopenmp -lrt
//usage: ./hashtable_omp <thread NUM>
typedef struct _list_t_ {
    char *str;
    struct _list_t_ *next;
} list_t;

typedef struct _hash_table_t_ {
    int size;       /* the size of the table */
    list_t **table; /* the table elements */
} hash_table_t;

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

void main(int argc, char const *argv[])
{
        int i;
        int findCnt;
        list_t*  listShow= NULL;
        
        char *find[]={"hello","home"};
        
        int wordsToHash = 20;//Remember to define your size!!!(how many phrase  is s[] )!

        static char *s[]={"steve","bOB","apple","ban","Johnson", 
        "banana","ice","happy","home","hello","love","wen","danny"
        ,"dog","hot" ,"cold","fato","fatrabbit","jerry","tux"};
        
        //timer
        hash_table_t *my_hash_table;
        clock_t         start, stop;
        struct timespec go, end;
        double cpu_time1;


        int size_of_table = 12;
        clock_gettime(CLOCK_REALTIME, &go);
        start = clock();
        my_hash_table = create_hash_table(size_of_table);
        
        thread_number = strtol(argv[1], NULL, 10);
        omp_set_num_threads(thread_number);
       #pragma omp parallel shared(wordsToHash,s,my_hash_table) private(i)
    {   
        //hashing   Remember
        for( i=0; i<wordsToHash ;i++){
        add_string(my_hash_table,s[i]);
        }
    }
        listShow = lookup_string(my_hash_table,find[1]);


        stop = clock();
        clock_gettime(CLOCK_REALTIME, &end);
        cpu_time1 = diff_in_second(go, end);
        float   elapsedTime = (float)(stop - start) /(float)CLOCKS_PER_SEC * 1000.0f;
        printf( "Time to hash:  %3.1f ms\n", elapsedTime );
        printf("execution time of cpu : %lf sec\n", cpu_time1);
        printf("Found the word list \n");
        printList(listShow);
        printf("Found the word is %s \n",listShow->str);
        printf("-------------------------- \n");  



        printf("Print hash table content \n");
        for(i=0; i<size_of_table ;i++)
        {
         printList(my_hash_table->table[i]);
        printf("\n");
        }

        free_table(my_hash_table);
}

/*
void verify_table( const Table &table ) {
    int count = 0;
    for (size_t i=0; i<table.count; i++) {
        Entry   *current = table.entries[i];
        while (current != NULL) {
            ++count;
            if (hash( current->key, table.count ) != i)
                printf( "%d hashed to %ld, but was located at %ld\n",
                        current->key,
                        hash( current->key, table.count ), i );
            current = current->next;
        }
    }
    if (count != ELEMENTS)
        printf( "%d elements found in hash table.  Should be %ld\n",
                count, ELEMENTS );
    else
        printf( "All %d elements found in hash table.\n", count);
}
*/