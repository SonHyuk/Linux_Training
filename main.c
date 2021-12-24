#if 1 // Modify
#include <stdio.h>
#include <string.h>

/*  fstat()  */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>


int main(int argc, char *argv[])
{
    /*  Read file  */
    int nFile = 0;
    char pBuffer_Contents[1024] = { 0, };

    unsigned int nFile_Count = 1;

    /*  fstat  */
    struct stat sb;
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;

    if( argc > 1 )
    {
        while( nFile_Count != argc )
        {
            nFile = open( argv[nFile_Count], O_RDONLY);
            if( nFile == -1 )
            {
                printf("> Error : Could not open file.\n");
                //close(nFile);
                nFile_Count++;
                continue;
            }
            else
            {
                if( stat(argv[nFile_Count], &sb) == -1 )
                {
                    perror("fstat error");
                    //return 0;
                }
            }

            //printf("nFile = %d\n", nFile);
            //printf("sb.st_mode = %x\n", (sb.st_mode & (S_IFMT)));


            if((sb.st_mode & (S_IFMT)) == S_IFDIR)
            {
                //printf("directory\n");

                if( (dir_ptr = opendir(argv[nFile_Count])) == NULL )
                {
                    printf("error\n");

                    return 0;
                }
                
                while( (file = readdir(dir_ptr)) != NULL )
                {
                    printf("%s\n", file->d_name);
                }
                
                //printf("\n");
                closedir(dir_ptr);
            }
            else
            {
                /*  Read and print  */
                while( read(nFile, pBuffer_Contents, 1023) > 0 )
                {
                    printf("%s",pBuffer_Contents);
                    memset( pBuffer_Contents, 0x00, sizeof(pBuffer_Contents) );
                }

                printf("\n");
                close(nFile);    
            }

            nFile_Count++;  
        }
    }
    else
    {
        printf("\n> Please enter a file name.\n");
    }
    
    return 0;
}

#else

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    /*  Read file  */
    FILE* pFile = NULL;
    char pBuffer_Contents[1024] = { 0, };

    unsigned int nFile_Count = 1;

    if( argc > 1 )
    {
        while( nFile_Count != argc )
        {
            if( (pFile = fopen( argv[nFile_Count], "rb") ) == NULL )
            {
                printf("> Error : Could not open file.\n");
                //fclose(pFile);
                nFile_Count++;
                continue;
            }

            //fseek( pFile, 0, SEEK_SET ); // Go to beginning of file

            /*  Read and print  */
            while( fread(pBuffer_Contents, 1, 1023, pFile) > 0 )
            {
                printf("%s",pBuffer_Contents);
                memset( pBuffer_Contents, 0x00, sizeof(pBuffer_Contents) );
            }

            printf("\n\n");

            fclose(pFile);

            nFile_Count++;
        }
    }
    else
    {
        printf("\n> Please enter a file name.\n");
    }

    return 0;
}

#endif