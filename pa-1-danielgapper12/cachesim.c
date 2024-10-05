#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#define ADDRESS_LENGTH 64  // 64-bit memory addressing

typedef struct
{
    int valid;
    unsigned long tag;
    int lru_counter; // least recently used
} CacheLine;

typedef struct
{
    CacheLine* lines;
} CacheSet;

typedef struct
{
    CacheSet* sets;
    int s; // number of set index bits
    int E; // number of lines per set
    int b; // number of bit blocks
} Cache;

void simulate_cache(Cache *cache, char operation, unsigned long long address, int *hit_count, int *miss_count, int *eviction_count, int verbose);
Cache* init_cache(int s, int E, int b);
void parse_tracefile(Cache *cache, const char *tracefile, int *hit_count, int *miss_count, int *eviction_count, int verbose);
void print_summary(int hits, int misses, int evictions);

/*
 * initializes the cache
 */ 
Cache* init_cache(int s, int E, int b)
{
    Cache *cache = (Cache*)malloc(sizeof(Cache));

    int numSets = pow(2, s); // number of sets = 2^s

    // allocate memory for the sets
    cache->sets = (CacheSet*)malloc(numSets * sizeof(CacheSet));

    // cache configuration parameters
    cache->E = E;
    cache->s = s;
    cache->b = b;

    // allocate memory for each cache line in each set
    for(int i = 0; i < numSets; i++) // goes through each set
    {
        cache->sets[i].lines = (CacheLine*)malloc(E * sizeof(CacheLine));
        // goes through each line with the set
        for(int j = 0; j < E; j++) 
        {
            cache->sets[i].lines[j].lru_counter = 0;
            cache->sets[i].lines[j].tag = 0;
            cache->sets[i].lines[j].valid = 0;
        }
    }
    return cache;
}

/*
 * simulates the cache
 */ 
void simulate_cache(Cache *cache, char operation, unsigned long long address, int *hit_count, int *miss_count, int *eviction_count, int verbose) 
{
    // get set index and tag, for set index, need to skip over offset
    int setIndex = (address >> cache->b) & ((1 << cache->s) - 1); 

    // tag is what is left over from set index and block offset
    unsigned long tag = address >> (cache->s + cache->b);
    CacheSet *set = &cache->sets[setIndex]; 

    int hit = 0;
    int emptyLine = -1;
    int evictLine = 0;
    int maxLRU = -1;

    // go through each line in the set
    for (int i = 0; i < cache->E; i++) 
    {
        // first, get line from the set
        CacheLine *line = &set->lines[i];
        
        // determine if hit or not, if line is valid and the tags match, then it's a hit 
        if (line->valid && line->tag == tag) 
        {
            (*hit_count)++;         
            line->lru_counter = 0;  
            hit = 1;
            if (verbose) 
            {
                printf("%c %llx,1 hit", operation, address); // don't add newline yet
            }
            break;
        }

        // keep track of the first empty line to be used to store data
        if (!line->valid && emptyLine == -1) 
        {
            emptyLine = i;
        }

        // get the line with the highest LRU to be evicted
        if (line->lru_counter > maxLRU) 
        {
            maxLRU = line->lru_counter;
            evictLine = i;
        }
    }

    // handles misses and evictions
    if (!hit) 
    {
        (*miss_count)++; 
        if (verbose) 
        {
            printf("%c %llx,1 miss", operation, address); // don't add newline yet
        }

        if (emptyLine != -1)
        {
            set->lines[emptyLine].valid = 1;
            set->lines[emptyLine].tag = tag;
            set->lines[emptyLine].lru_counter = 0;
        } 
        else 
        {
            (*eviction_count)++; 
            set->lines[evictLine].tag = tag;
            set->lines[evictLine].lru_counter = 0;
            if (verbose) 
            {
                printf(" eviction"); 
            }
        }
    }

    // update least recently used counters for every line that is valid 
    for (int i = 0; i < cache->E; i++) 
    {
        if (set->lines[i].valid) 
        {
            set->lines[i].lru_counter++;
        }
    }

    // take care of extra hit if it's a modify operation
    if (operation == 'M') 
    {
        (*hit_count)++;
        if (verbose) 
        {
            printf(" hit"); // append "hit" to current line
        }
    }

    if (verbose) 
    {
        printf("\n");
    }
}

/*
 * parse trace file
 */ 
void parse_tracefile(Cache *cache, const char *tracefile, int *hit_count, int *miss_count, int *eviction_count, int verbose)
{
    FILE *file = fopen(tracefile, "r");
    if (!file)
    {
        printf("Error opening trace file\n");
        exit(1);
    }

    char operation;
    unsigned long long address;
    int size;

    // read each line from the trace file
    while (fscanf(file, " %c %llx,%d", &operation, &address, &size) > 0)
    {
        if (operation == 'I')
        {
            continue; // if instruction load then ignore
        }
        simulate_cache(cache, operation, address, hit_count, miss_count, eviction_count, verbose);
    }
    fclose(file);
}


/*
 * this function provides a standard way for your cache
 * simulator to display its final statistics (i.e., hit and miss)
 */
void print_summary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
}

/*
 * print usage info
 */
void print_usage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/trace01.dat\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/trace01.dat\n", argv[0]);
    exit(0);
}

int main(int argc, char* argv[])
{
    int c = 0;
    char* tracefile = NULL;
    int s = 0, E = 0, b = 0;
    int verbose = 0;
    int hit_count = 0;
    int miss_count = 0;
    int eviction_count = 0;

    // "s:E:b:t:vh" specifies all switch cases, if followed by ":" that signifies argument expected
    while( (c = getopt(argc, argv, "s:E:b:t:vh")) != -1)
    {
        switch(c)
        {
            case 's':
                s = atoi(optarg); // number of set index bits
                break;
            case 'E':
                E = atoi(optarg); // number of lines per set
                break;
            case 'b':
                b = atoi(optarg); // number of block bits
                break;
            case 't':
                tracefile = optarg; // gets tracefile name
                break;
            case 'v': // optional verbose flag that prints trace info
                verbose = 1;
                break;
            case 'h': // optional flag that prints usage info
                print_usage(argv);
                exit(0);
            default:
                print_usage(argv);
                exit(1);
        }
    }
    // catch errors
    if(!tracefile || s == 0 || E == 0 || b ==0)
    {
        print_usage(argv);
        exit(1);
    }
    if(!verbose)
    {
        printf("(s,E,b)=(%d,%d,%d)\n", s, E, b);
        printf("Trace: %s\n", tracefile);
    }

    // initialize the cache
    Cache *cache = init_cache(s, E, b);

    // parse through the trace file and simulate the cache
    parse_tracefile(cache, tracefile, &hit_count, &miss_count, &eviction_count, verbose);

    // output cache hit and miss statistics
    print_summary(hit_count, miss_count, eviction_count);

    // free allocated memory
    for(int i = 0; i < pow(2, s); i++)
    {
        free(cache->sets[i].lines);
    }

    free(cache->sets);
    free(cache);

    return 0;
}
