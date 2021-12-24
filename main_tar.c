
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <zlib.h>


#define BUFFER_SIZE     (1024)
#define READ_SIZE       (128)

#define TAR_OPTION_Z    (1 << 0)
#define TAR_OPTION_X    (1 << 1)
#define TAR_OPTION_FLAG (1 << 2)
#define TAR_OPTION_C    (1 << 3)
#define TAR_OPTION_N    (1 << 4)

#define OPTION_C_DEFAULT_FILE_NAME  ("Option_c_test")
#define OPTION_Z_DEFAULT_FILE_NAME  ("Option_z_test")


int Get_Option(int argc, char **argv);


int main(int argc, char *argv[])
{
    unsigned int nFile_Count = 2; // option : 1
    char pBuffer_tar[BUFFER_SIZE] = { 0, };

    int nFile_out = 0;
    int nFile_arg = 0;
    int nFile_temp = 0;

    unsigned int nFile_Size = 0;
    unsigned int nWrite_Size = 0;
    unsigned int nActual_Read_Size = 0;
    unsigned int nRead_Size = READ_SIZE;
    unsigned int nContentsStartPoint = 0;

    char *pFirst_Mark_Address = NULL;
    char *pSecond_Mark_Address = NULL;
    char pFound_File_Name[1024] = { 0, };
    char pFound_File_Size[1024] = { 0, };

    int nOption = 0;
    bool bReadEnd_Flag = false;

    gzFile zfp = NULL;
    char z_buffer[BUFFER_SIZE] = { 0, };
    int nOption_z_Read_Size = 0;
    char pOption_z_File_Name[1024] = { 0, };

    struct stat sb;


    if( argc > 1 )
    {
        nOption = Get_Option(argc, argv);
        if( nOption == 0 )
        {
            return 0;
        }

        if( nOption >= (TAR_OPTION_C + TAR_OPTION_N) )
        {
            if( nOption & TAR_OPTION_Z )
            {
                sprintf(pOption_z_File_Name, "%s", argv[2]);

                nFile_out = open("Option_z_temp_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
                if( nFile_out == -1 )
                {
                    printf("> [nFile_out] Error : Could not open file.\n");
                    return 0;
                }
            }
            else
            {
                nFile_out = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);
                if( nFile_out == -1 )
                {
                    printf("> [nFile_out] Error : Could not open file.\n");
                    return 0;
                }
            }

            nFile_Count++;
        }
        else if (nOption & TAR_OPTION_C)
        {
            if( argc == 2 )
            {
                printf("> Please enter option & file name.\n");
                return 0;
            }

            if( nOption & TAR_OPTION_Z )
            {
                nFile_out = open("Option_z_temp_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
            }
            else
            {
                nFile_out = open(OPTION_C_DEFAULT_FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
            }

            if( nFile_out == -1 )
            {
                printf("> [nFile_out] Error : Could not open file.\n");
                return 0;
            }
        }

        while( nFile_Count != argc )
        {
            if( nOption > TAR_OPTION_FLAG ) // Option c
            {
                nFile_arg = open( argv[nFile_Count], O_RDONLY);
                if( nFile_arg == -1 )
                {
                    printf("> [nFile_arg] Error : Could not open file.\n");
                    nFile_Count++;
                    continue;
                }

                if( stat(argv[nFile_Count], &sb) == -1 )
                {
                    perror("fstat error\n");
                }

                /*  "file name" + "\" at the beginning of file  */
                sprintf( pBuffer_tar, "%s\\", argv[nFile_Count] );
                nWrite_Size += strlen(pBuffer_tar); // File name

                //lseek( nFile_arg, 0, SEEK_END ); // Go to end of file
                //nFile_Size = ftell(nFile_arg); // Check file size
                //lseek( nFile_arg, 0, SEEK_SET ); // Go to beginning of file
                nFile_Size = sb.st_size; // Check file size

                /*  "file name" + "\" + "file size" + "\"  */
                sprintf( pBuffer_tar + nWrite_Size, "%d\\", nFile_Size );      
                nWrite_Size += strlen(pBuffer_tar + nWrite_Size); // File size  

                /*  nFile_Size = strlen("file name" + "\" + "file size" + "\") + original file size  */
                nFile_Size += nWrite_Size;

                while( ( nActual_Read_Size = read(nFile_arg, pBuffer_tar + nWrite_Size, READ_SIZE) ) > 0 )
                {                
                    nWrite_Size += nActual_Read_Size;

                    //printf("nWrite_Size[%d], nFile_Size[%d], nActual_Read_Size[%d]\n", nWrite_Size, nFile_Size, nActual_Read_Size);

                    if(    0
                        || ((nWrite_Size + READ_SIZE) > BUFFER_SIZE)
                        || ( nWrite_Size == nFile_Size )
                    )
                    {
                        //printf("@@@@@@@@@@ Write @@@@@@@@@@@\n");

                        write( nFile_out, pBuffer_tar, nWrite_Size );
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) ); // Init buffer
                        nFile_Size -= nWrite_Size;
                        nWrite_Size = 0;
                    }
                }            

                nFile_Count++;         
                close(nFile_arg);   
            }
            else if( nOption < TAR_OPTION_FLAG ) // Option x
            {
                nContentsStartPoint = 0; // initialize when file change

                if( nOption & TAR_OPTION_Z ) // Option x + z
                {
                    zfp = gzopen( argv[nFile_Count], "rb" );
                    if( zfp == NULL )
                    {
                        printf("> Error : gzopen error\n");
                        return 0;
                    }

                    nFile_temp = open("Option_z_temp_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
                    if( nFile_temp == -1 )
                    {
                        printf("> [nFile_temp] Error : Could not open file.\n");
                        return 0;
                    }

                    while( gzgets( zfp, z_buffer, BUFFER_SIZE ) != NULL ) // first read
                    {
                        write( nFile_temp, z_buffer, strlen(z_buffer) );
                    }

                    lseek( nFile_temp, 0, SEEK_SET ); // Go to beginning of file

                    while(1)
                    {
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );

                        /*  Read file to find file name, file size  */
                        if( !(read(nFile_temp, pBuffer_tar, 512)) )
                        {
                            /*  when next file not exist  */
                            break;
                        }

                        memset( pFound_File_Name, 0x00, sizeof(pFound_File_Name) );
                        memset( pFound_File_Size, 0x00, sizeof(pFound_File_Size) );

                         /*  Find file name  */
                        pFirst_Mark_Address = strstr( pBuffer_tar, "\\" );
                        if( pFirst_Mark_Address == NULL )
                        {
                            printf("> Error : Invalid target file - First\n");
                            break;
                        }
                        memcpy( pFound_File_Name, pBuffer_tar, pFirst_Mark_Address - pBuffer_tar );
                        printf("pFound_File_Name[%s]\n", pFound_File_Name);

                        /*  Find file size  */
                        pSecond_Mark_Address = strstr( pFirst_Mark_Address+1, "\\" );
                        if( pSecond_Mark_Address == NULL )
                        {
                            printf("> Error : Invalid target file - Second\n");
                            break;
                        }
                        memcpy( pFound_File_Size, pFirst_Mark_Address+1, pSecond_Mark_Address - (pFirst_Mark_Address+1) );
                        printf("pFound_File_Size[%s]\n", pFound_File_Size);

                        nFile_out = open(pFound_File_Name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if( nFile_out == -1 )
                        {
                            printf("> [nFile_out] Error : Could not open file.\n");
                            return 0;
                        }

                        nFile_Size = atoi( pFound_File_Size ); // nFile_Size = pFound_File_Size (char* --> int)
                        //printf("nFile_Size[%d]\n", nFile_Size);

                        lseek( nFile_temp, 0, SEEK_SET ); // Go to beginning of file
                        nContentsStartPoint += pSecond_Mark_Address - pBuffer_tar + 1;
                        //printf("nContentsStartPoint[%d]\n", nContentsStartPoint);
                        lseek( nFile_temp, nContentsStartPoint, SEEK_CUR ); // Go to contents start point of file
                        
                        nContentsStartPoint += nFile_Size; // for next create file
                        
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );

                        if( nRead_Size > nFile_Size )
                        {
                            nRead_Size = nFile_Size;
                        }
                        
                        while( (nActual_Read_Size = read(nFile_temp, pBuffer_tar + nWrite_Size, nRead_Size)) > 0 )
                        {
                            nWrite_Size += nActual_Read_Size;

                            // check next reading         
                            if( nWrite_Size + READ_SIZE >= nFile_Size )
                            {
                                nRead_Size = nFile_Size - nWrite_Size;
                                bReadEnd_Flag = true;
                            }

                            //printf("nWrite_Size[%d], nFile_Size[%d], nActual_Read_Size[%d], nRead_Size[%d]\n", nWrite_Size, nFile_Size, nActual_Read_Size, nRead_Size);

                            if(    (nWrite_Size >= BUFFER_SIZE)
                                || (nFile_Size == nWrite_Size)
                            )
                            {
                                //printf("@@@@@@@@@@ Write @@@@@@@@@@@\n");
                                
                                write( nFile_out, pBuffer_tar, nWrite_Size );
                                memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
                                nFile_Size -= nWrite_Size;
                                nWrite_Size = 0;
                                nRead_Size = READ_SIZE;

                                if( bReadEnd_Flag )
                                {
                                    bReadEnd_Flag = false;
                                    close(nFile_out);

                                    break;
                                }
                            }
                        }
                    }

                    close(nFile_temp);

                    remove("Option_z_temp_file");

                    //gzclose(zfp);
                }
                else // Option x
                {
                    nFile_arg = open( argv[nFile_Count], O_RDONLY);
                    if( nFile_arg == -1 )
                    {
                        printf("> [nFile_arg] Error : Could not open file.\n");
                        nFile_Count++;
                        continue;
                    }    

                    while(1)
                    {
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );

                        /*  Read file to find file name, file size  */
                        if( !(read(nFile_arg, pBuffer_tar, 512)) )
                        {
                            /*  when next file not exist  */
                            break;
                        }
                        
                        memset( pFound_File_Name, 0x00, sizeof(pFound_File_Name) );
                        memset( pFound_File_Size, 0x00, sizeof(pFound_File_Size) );

                        /*  Find file name  */
                        pFirst_Mark_Address = strstr( pBuffer_tar, "\\" );
                        if( pFirst_Mark_Address == NULL )
                        {
                            printf("> Error : Invalid target file\n");
                            break;
                        }
                        memcpy( pFound_File_Name, pBuffer_tar, pFirst_Mark_Address - pBuffer_tar );
                        printf("pFound_File_Name[%s]\n", pFound_File_Name);

                        /*  Find file size  */
                        pSecond_Mark_Address = strstr( pFirst_Mark_Address+1, "\\" );
                        if( pSecond_Mark_Address == NULL )
                        {
                            printf("> Error : Invalid target file\n");
                            break;
                        }
                        memcpy( pFound_File_Size, pFirst_Mark_Address+1, pSecond_Mark_Address - (pFirst_Mark_Address+1) );
                        printf("pFound_File_Size[%s]\n", pFound_File_Size);

                        nFile_out = open(pFound_File_Name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if( nFile_out == -1 )
                        {
                            printf("> [nFile_out] Error : Could not open file.\n");
                            return 0;
                        }

                        nFile_Size = atoi( pFound_File_Size ); // nFile_Size = pFound_File_Size (char* --> int)
                        //printf("nFile_Size[%d]\n", nFile_Size);

                        lseek( nFile_arg, 0, SEEK_SET ); // Go to beginning of file
                        nContentsStartPoint += pSecond_Mark_Address - pBuffer_tar + 1;
                        //printf("nContentsStartPoint[%d]\n", nContentsStartPoint);
                        lseek( nFile_arg, nContentsStartPoint, SEEK_CUR ); // Go to contents start point of file
                        
                        nContentsStartPoint += nFile_Size; // for next create file
                        
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );

                        if( nRead_Size > nFile_Size )
                        {
                            nRead_Size = nFile_Size;
                        }

                        while( (nActual_Read_Size = read(nFile_arg, pBuffer_tar + nWrite_Size, nRead_Size)) > 0 )
                        {
                            nWrite_Size += nActual_Read_Size;

                            // check next reading         
                            if( nWrite_Size + READ_SIZE >= nFile_Size )
                            {
                                nRead_Size = nFile_Size - nWrite_Size;
                                bReadEnd_Flag = true;
                            }

                            //printf("nWrite_Size[%d], nFile_Size[%d], nActual_Read_Size[%d], nRead_Size[%d]\n", nWrite_Size, nFile_Size, nActual_Read_Size, nRead_Size);

                            if(    (nWrite_Size >= BUFFER_SIZE)
                                || (nFile_Size == nWrite_Size)
                            )
                            {
                                //printf("@@@@@@@@@@ Write @@@@@@@@@@@\n");
                                
                                write( nFile_out, pBuffer_tar, nWrite_Size );
                                memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
                                nFile_Size -= nWrite_Size;
                                nWrite_Size = 0;
                                nRead_Size = READ_SIZE;

                                if( bReadEnd_Flag )
                                {
                                    bReadEnd_Flag = false;
                                    close(nFile_out);
                                    break;
                                }
                            }
                        }
                    }
                }

                nFile_Count++;         
                close(nFile_arg);   
            }
        }

        if( nOption & TAR_OPTION_Z )
        {
            gzclose(zfp);
        }

        if( nOption & TAR_OPTION_C )
        {
#if 1 // zlib test            
            if( nOption & TAR_OPTION_Z )
            {
                lseek( nFile_out, 0, SEEK_SET ); // Go to beginning of file

                if( nOption & TAR_OPTION_N )
                {
                    zfp = gzopen(pOption_z_File_Name, "wb");
                }
                else
                {
                    zfp = gzopen(OPTION_Z_DEFAULT_FILE_NAME, "wb");
                }
                
                if( zfp == NULL )
                {
                    printf("> Error : gzopen error\n");
                    return 0;
                }

                while( (nOption_z_Read_Size = read(nFile_out, z_buffer, BUFFER_SIZE)) > 0 )
                {
                    if( gzwrite(zfp, z_buffer, nOption_z_Read_Size) < 0 )
                    {
                        printf("> Error : gzwrite error\n");
                        return 0;
                    }
                }
                
                gzclose(zfp);

                remove("Option_z_temp_file");
            }
#endif            
            close(nFile_out);
        }
    }
    else
    {
        printf("\n> Please enter option & file name.\n");
    }
}



/*  function part  */

int Get_Option(int argc, char **argv)
{
    int nGetOption = 0;
    int nOption = 0;

    while( (nGetOption = getopt(argc, argv, "xcnz")) != -1 )
    {
        switch( nGetOption )
        {
            case 'x':
                if(     (nOption & TAR_OPTION_C)
                    ||  (nOption & TAR_OPTION_N)
                )
                {
                    printf("> Error : option\n");
                    return 0;
                }
                else
                {
                    if( !(nOption & TAR_OPTION_X) )
                    {
                        nOption += TAR_OPTION_X;
                    }
                }
                break;

            case 'c':
                if( nOption & TAR_OPTION_X )
                {
                    printf("> Error : option\n");
                    return 0;
                }
                else
                {
                    if( !(nOption & TAR_OPTION_C) )
                    {
                        nOption += TAR_OPTION_C;
                    }
                }
                break;

            case 'n':
                if( nOption & TAR_OPTION_X )
                {
                    printf("> Error : option\n");
                    return 0;
                }
                else
                {
                    if( !(nOption & TAR_OPTION_N) )
                    {
                        nOption += TAR_OPTION_N;
                    }
                }
                break;

            case 'z':
                if( !(nOption & TAR_OPTION_Z) )
                {
                    nOption += TAR_OPTION_Z;
                }
                break;
        }
    }

    return nOption;
}