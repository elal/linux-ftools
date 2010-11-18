#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux-ftools.h>
#include <math.h>
#include <errno.h>

char STR_FORMAT[] =  "%-80s %15s %15s %15s %15s %15s %15s %15s\n";
char DATA_FORMAT[] = "%-80s %15ld %15d %15d %15d %15f %15d %15d %15d %15d %15d %15d %15d %15d %15d %15d\n";

struct fincore_result 
{
    long cached_size;
};

char *_itoa(int n) {
	static char retbuf[100];
	sprintf(retbuf, "%d", n);
	return retbuf;
}

char *_ltoa( long value ) {

    static char buff[100];

    sprintf( buff, "%ld", value );

    return buff;

}

char *_dtoa( double value ) {

    static char buff[100];

    sprintf( buff, "%f", value );

    return buff;

}

void fincore(char* path, 
             int pages, 
             int summarize, 
             int only_cached, 
             struct fincore_result *result, 
             char* format ) {

    int fd;
    struct stat file_stat;
    void *file_mmap;
    unsigned char *mincore_vec;
    size_t page_size = getpagesize();
    size_t page_index;

    int flags = O_RDWR;
    
    //TODO:
    //
    // pretty print integers with commas... 
    fd = open(path,flags);

    if ( fd == -1 ) {

        perror( "can not open file" );
        fprintf( stderr, "%s: can not open file: %s\n", path, strerror(errno) );

        return;
    }

    if ( fstat( fd, &file_stat ) < 0 ) {

        perror( sprintf( "%s: Could not stat file", path ) );

        goto cleanup;

    }

    file_mmap = mmap((void *)0, file_stat.st_size, PROT_NONE, MAP_SHARED, fd, 0);
    
    if ( file_mmap == MAP_FAILED ) {
        perror( sprintf( "%s: Could not mmap file", path ) );
        goto cleanup;      
    }

    mincore_vec = calloc(1, (file_stat.st_size+page_size-1)/page_size);

    if ( mincore_vec == NULL ) {
        //something is really wrong here.  Just exit.
        perror( "Could not calloc" );
        exit( 1 );
    }

    if ( mincore(file_mmap, file_stat.st_size, mincore_vec) != 0 ) {
        perror( sprintf( "%s: Could not call mincore on file", path ) );
        exit( 1 );
    }

    int total_pages = (int)ceil( (double)file_stat.st_size / (double)page_size );

    int cached = 0;
    int printed = 0;

    int r0 = 0;
    int r1 = 0;
    int r2 = 0;
    int r3 = 0;
    int r4 = 0;
    int r5 = 0;
    int r6 = 0;
    int r7 = 0;
    int r8 = 0;
    int r9 = 0;

    //printf( "total_pages: %d", total_pages );
    fflush( stdout );

    int ptr = (int)(total_pages / (float)10);

    //r0.nr_pages = 

    for (page_index = 0; page_index <= file_stat.st_size/page_size; page_index++) {

        if (mincore_vec[page_index]&1) {
            ++cached;

            if ( pages ) {
                printf("%lu ", (unsigned long)page_index);
                ++printed;
            }

            if ( ptr > 0 ) {
                int region = (page_index / ptr ) - 1;

                switch( region ) 
                    {
                    case 0:
                        ++r0;
                    case 1:
                        ++r1;
                    case 2:
                        ++r2;
                    case 3:
                        ++r3;
                    case 4:
                        ++r4;
                    case 5:
                        ++r5;
                    case 6:
                        ++r6;
                    case 7:
                        ++r7;
                    case 8:
                        ++r8;
                    case 9:
                        ++r9;
                    }
            }
        }

    }

    if ( printed ) printf("\n");

    // TODO: make all these variables long and print them as ld

    double cached_perc = 100 * (cached / (double)total_pages); 

    long cached_size = (double)cached * (double)page_size;

    if ( only_cached == 0 || cached > 0 ) {

        printf( DATA_FORMAT, 
                path, 
                file_stat.st_size , 
                total_pages , 
                cached ,  
                cached_size , 
                cached_perc ,
                r0 ,
                r1 , 
                r2 , 
                r3 , 
                r4 , 
                r5 , 
                r6 ,
                r7 ,
                r8 ,
                r9 );

    }

    result->cached_size = cached_size;

 cleanup:

    if ( mincore_vec != NULL )
        free(mincore_vec);

    if ( file_mmap != MAP_FAILED )
        munmap(file_mmap, file_stat.st_size);

    if ( fd != -1 )
        close(fd);

    return;

}

// print help / usage
void help() {

    fprintf( stderr, "%s version %s\n", "fincore", LINUX_FTOOLS_VERSION );
    fprintf( stderr, "fincore [options] files...\n" );
    fprintf( stderr, "\n" );

    fprintf( stderr, "  --pages=true|false    Don't print pages\n" );
    fprintf( stderr, "  --summarize           When comparing multiple files, print a summary report\n" );
    fprintf( stderr, "  --only-cached         Only print stats for files that are actually in cache.\n" );

}

/**
 * see README
 */
int main(int argc, char *argv[]) {

    if ( argc == 1 ) {
        help();
        exit(1);
    }

    //keep track of the file index.
    int fidx = 1;

    // parse command line options.

    //TODO options:
    //
    // --pages=true|false print/don't print pages
    // --summarize when comparing multiple files, print a summary report
    // --only-cached only print stats for files that are actually in cache.

    int i = 1; 

    int pages         = 0;
    int summarize     = 1;
    int only_cached   = 0;

    for( ; i < argc; ++i ) {

        if ( strcmp( "--pages=false" , argv[i] ) == 0 ) {
            pages = 0;
            ++fidx;
        }

        if ( strcmp( "--pages=true" , argv[i] ) == 0 ) {
            pages = 1;
            ++fidx;
        }

        if ( strcmp( "--summarize" , argv[i] ) == 0 ) {
            summarize = 1;
            ++fidx;
        }

        if ( strcmp( "--only-cached" , argv[i] ) == 0 ) {
            only_cached = 1;
            ++fidx;
        }

        if ( strcmp( "--help" , argv[i] ) == 0 ) {
            help();
            exit(1);
        }

        //TODO what if this starts -- but we don't know what option it is?

    }

    long total_cached_size = 0;

    char format[] = "%-80s %15s %15s %15s %15s %15s %15s %15s\n";

    printf( format, "filename", "size", "total_pages", "cached_pages", "cached_size", "cached_perc", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9" );
    printf( format, "--------", "----", "-----------", "------------", "-----------", "-----------", "--", "--" , "--", "--" , "--", "--" , "--", "--" , "--", "--" );

    for( ; fidx < argc; ++fidx ) {

        char* path = argv[fidx];

        struct fincore_result result;

        fincore( path, pages, summarize, only_cached , &result, &format );

        total_cached_size += result.cached_size;

    }
    
    if ( summarize ) {
        
        printf( "---\n" );

        //TODO: add more metrics including total size... 
        printf( "total cached size: %ld\n", total_cached_size );

    }

}
