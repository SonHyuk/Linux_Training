
#include "./tar_function.h"



int main(int argc, char *argv[])
{
    unsigned int nFile_Count = 2; // option : 1
    char pBuffer_tar[BUFFER_SIZE] = { 0, };

    int nFile_out = 0;
    int nFile_arg = 0;
    int nFile_temp = 0;
    int nFile_option_z = 0;
    int nReturn_Value = 0;

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

    char pBackup_Working_Directory[100] = { 0, };
    unsigned short nDirectory_Cnt = 0;

#if 1 // struct test
    File_Info File_Info;
    memset( &File_Info, 0x00, sizeof(File_Info) );
#endif

#if 1 // compress, uncompress test

    Byte deflate_data[DBUF] = { 0, };
    uLong deflate_size = DBUF;

    Byte inflate_data[BUF] = { 0, };
    uLong inflate_size = BUF;

    // test
    int nTest_Return_Value = 0;
#endif


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
                nFile_option_z = open("Option_z_test_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
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

        getcwd(pBackup_Working_Directory, 100);
                            
        while( nFile_Count != argc )
        {
#if 1 // struct test            
            // initialize struct
            //memset( &File_Info, 0x00, sizeof(File_Info) );
#endif            

            if( nOption > TAR_OPTION_FLAG ) // Option c
            {
#if 1 // function test
                chdir(pBackup_Working_Directory);

                if( stat(argv[nFile_Count], &sb) == -1 )
                {
                    perror("fstat error_main\n");
                }

                if( Check_File_Type(sb) ) // directory
                {
                    nDirectory_Cnt++;

                    Create_Archive_Directory( argv[nFile_Count], nFile_out, 0, NULL );   

                    nReturn_Value = Check_Directory( argv[nFile_Count], nFile_out, 1 );
                    if( nReturn_Value == -1 ) // Error opening directory
                    {
                        nFile_Count++;
                        continue;
                    }
                }
                else // file
                {
                    nReturn_Value = Create_Archive(argv[nFile_Count], nFile_out, (nOption & TAR_OPTION_Z), 0, NULL);
                    if( nReturn_Value == -1 )
                    {
                        nFile_Count++;
                        continue;
                    }
                }

                nFile_Count++;

#else
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

#if 1 // struct test
                // file name
                snprintf( File_Info.pName, 128, "%s", argv[nFile_Count] );
                // file size
                File_Info.nSize = sb.st_size;
                nFile_Size = File_Info.nSize;

                memcpy( pBuffer_tar, &File_Info, sizeof(File_Info) );

                write( nFile_out, pBuffer_tar, sizeof(File_Info) );
                memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
#else
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
#endif

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
#endif                
            }
            else if( nOption < TAR_OPTION_FLAG ) // Option x
            {
                nContentsStartPoint = 0; // initialize when file change

                if( nOption & TAR_OPTION_Z ) // Option x + z
                {
#if 1 // compress test                    
                    nFile_arg = open( argv[nFile_Count], O_RDONLY);
                    if( nFile_arg == -1 )
                    {
                        printf("> [nFile_arg] Error : Could not open file.\n");
                        nFile_Count++;
                        continue;
                    }

                    nFile_temp = open("Option_z_temp_file", O_RDWR | O_CREAT | O_TRUNC, 0644);
                    if( nFile_temp == -1 )
                    {
                        printf("> [nFile_temp] Error : Could not open file.\n");
                        return -1;
                    }

                    if( stat(argv[nFile_Count], &sb) == -1 )
                    {
                        perror("fstat error\n");
                    }

                    // file size
                    nFile_Size = sb.st_size;

                    // if( nRead_Size > nFile_Size )
                    // {
                    //     nRead_Size = nFile_Size;
                    // }
                    
                    while( (nActual_Read_Size = read(nFile_arg, pBuffer_tar + nWrite_Size, nRead_Size)) > 0 )
                    {

                        nWrite_Size += nActual_Read_Size;

                        // check next reading         
                        if( nWrite_Size + READ_SIZE >= nFile_Size )
                        {
                            nRead_Size = nFile_Size - nWrite_Size;
                            bReadEnd_Flag = true;
                        }

                        printf("nWrite_Size[%d], nFile_Size[%d], nActual_Read_Size[%d], nRead_Size[%d]\n", nWrite_Size, nFile_Size, nActual_Read_Size, nRead_Size);

                        if(    (nWrite_Size >= BUFFER_SIZE)
                            || (nFile_Size == nWrite_Size)
                        )
                        {
                            printf("@@@@@@@@@@ Temp File Write @@@@@@@@@@@\n");
                            
                            // test
                            uncompress(inflate_data, &inflate_size, pBuffer_tar, nWrite_Size);
                            //nTest_Return_Value = uncompress(inflate_data, &inflate_size, pBuffer_tar, nWrite_Size);

                            //nTest_Return_Value = uncompress(inflate_data, &inflate_size, pBuffer_tar, nWrite_Size);

                            printf("sizeof(inflate_data)[%d]\n", sizeof(inflate_data));
                            printf("nTest_Return_Value[%d]\n", nTest_Return_Value);

                            // test
                            printf("nWrite_Size[%d]\n", nWrite_Size);           
                            printf("inflate_size[%d]\n", inflate_size);           

                            write( nFile_temp, inflate_data, inflate_size );

                            // test
                            memset( inflate_data, 0x00, sizeof(inflate_data) );
                            inflate_size = BUF;

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
#else
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
                    
#endif                    
                    // test
                    //return -1;
                    
                    lseek( nFile_temp, 0, SEEK_SET ); // Go to beginning of file

                    while(1)
                    {
                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );

#if 1 // struct test
                        // struct
                        if( !(read(nFile_temp, pBuffer_tar, sizeof(File_Info))) )
                        {
                            /*  when next file not exist  */
                            break;
                        }

                        memcpy( &File_Info, pBuffer_tar, sizeof(File_Info) );

                        // test
                        printf("File Name : %s\n", File_Info.pName);
                        printf("File Size : %d\n", File_Info.nSize);

                        nFile_out = open(File_Info.pName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if( nFile_out == -1 )
                        {
                            printf("> [nFile_out] Error : Could not open file.\n");
                            return -1;
                        }

                        nFile_Size = File_Info.nSize;

                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
#else
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
#endif

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
                    //getcwd(pBackup_Working_Directory, 100);

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

#if 1 // struct test
                        // struct
                        if( !(read(nFile_arg, pBuffer_tar, sizeof(File_Info))) )
                        {
                            /*  when next file not exist  */
                            break;
                        }

                        memcpy( &File_Info, pBuffer_tar, sizeof(File_Info) );

                        if( File_Info.nDirectory_Depth == 0 )
                        {
                            chdir(pBackup_Working_Directory);
                        }

                        // directory
                        if(     (File_Info.nKind == KIND_DIRECTORY)
                            ||  (File_Info.nDirectory_Depth != 0)
                        )
                        {
                            nReturn_Value = Extract_Archive_Directory( &File_Info );
                            if( nReturn_Value == KIND_DIRECTORY )
                            {
                                continue;                                
                            }
                        }
                        // else if( File_Info.nDirectory_Depth == 0 )
                        // {
                        //     chdir(pBackup_Working_Directory);
                        // }

                        // test
                        printf("File Name : %s\n", File_Info.pName);
                        printf("File Size : %d\n", File_Info.nSize);

                        nFile_out = open(File_Info.pName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if( nFile_out == -1 )
                        {
                            printf("> [nFile_out] Error : Could not open file.\n");
                            return -1;
                        }

                        nFile_Size = File_Info.nSize;

                        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
#else
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
#endif

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

                // if( nOption & TAR_OPTION_N )
                // {
                //     zfp = gzopen(pOption_z_File_Name, "wb");
                // }
                // else
                // {
                //     zfp = gzopen(OPTION_Z_DEFAULT_FILE_NAME, "wb");
                // }
                
                // if( zfp == NULL )
                // {
                //     printf("> Error : gzopen error\n");
                //     return 0;
                // }

                while( (nOption_z_Read_Size = read(nFile_out, z_buffer, BUFFER_SIZE)) > 0 )
                {
#if 1 // Modify zip
                    printf("nOption_z_Read_Size : %d\n", nOption_z_Read_Size);

                    compress(deflate_data, &deflate_size, z_buffer, nOption_z_Read_Size);

                    // test
                    printf("deflate_size : %d\n", deflate_size);

                    write(nFile_option_z, deflate_data, deflate_size);
                    
                    // test
                    //uncompress(inflate_data, &inflate_size, deflate_data, deflate_size);
                    //printf("inflate_data : %s\n", inflate_data);

                    memset( z_buffer, 0x00, sizeof(z_buffer) );
                    memset( deflate_data, 0x00, sizeof(deflate_data) );
                    memset( inflate_data, 0x00, sizeof(inflate_data) );
#else
                    if( gzwrite(zfp, z_buffer, nOption_z_Read_Size) < 0 )
                    {
                        printf("> Error : gzwrite error\n");
                        return 0;
                    }

                    memset( z_buffer, 0x00, sizeof(z_buffer) );
#endif                    
                }
                
                //gzclose(zfp);

                remove("Option_z_temp_file");
            }
#endif            
            close(nFile_out);
            close(nFile_option_z);
        }
    }
    else
    {
        printf("\n> Please enter option & file name.\n");
    }
}
