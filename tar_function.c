
#include "./tar_function.h"


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


int Create_Archive(char *pFile_Name, int nFile_out, bool Option_z, unsigned short nDirecotry_Depth, char *pPrev_Path)
{
    int nFile_arg = 0;

    unsigned int nFile_Size = 0;
    unsigned int nActual_Read_Size = 0;
    unsigned int nWrite_Size = 0;

    char pBuffer_tar[BUFFER_SIZE] = { 0, };

    // test
    char pInfo_Buffer[BUFFER_SIZE] = { 0, };

    struct stat sb;

#if 1 // compress, uncompress test
    Byte deflate_data[DBUF] = { 0, };
    uLong deflate_size = DBUF;

    Byte inflate_data[BUF] = { 0, };
    uLong inflate_size = BUF;
#endif    

    File_Info File_Info;
    memset( &File_Info, 0x00, sizeof(File_Info) );

    // mkdir test
    //mkdir("./test_test/directory_test", S_IRWXU);

    if( stat(pFile_Name, &sb) == -1 )
    {
        perror("fstat error\n");
    }


    nFile_arg = open( pFile_Name, O_RDONLY);
    if( nFile_arg == -1 )
    {
        printf("> [nFile_arg] Error : Could not open file.\n");
        //(*nFile_Count)++;
        //continue;
        return -1;
    }


    // file name
    snprintf( File_Info.pName, sizeof(File_Info.pName), "%s", pFile_Name );

    //memcpy( File_Info.pName, argv[nFile_Count], strlen(argv[nFile_Count]) );
    // file size
    File_Info.nSize = sb.st_size;
    nFile_Size = File_Info.nSize;

    File_Info.nDirectory_Depth = nDirecotry_Depth;
    snprintf( File_Info.pPrevPath, sizeof(File_Info.pPrevPath), "%s", pPrev_Path );


#if 0 // compress test
    if( Option_z )
    {
        //memcpy( pInfo_Buffer, &File_Info, sizeof(File_Info) );
        
        //compress(deflate_data, &deflate_size, pInfo_Buffer, sizeof(File_Info) );
        //nWrite_Size = deflate_size;
        //memcpy(pBuffer_tar, deflate_data, deflate_size);

        memcpy( pBuffer_tar, &File_Info, sizeof(File_Info) );

        nFile_Size += sizeof(File_Info);
        nWrite_Size += sizeof(File_Info);

        //write( nFile_out, deflate_data, deflate_size );
        //memset( deflate_data, 0x00, sizeof(deflate_data) ); // Init buffer
        //deflate_size = DBUF;
    }
    else
    {
        memcpy( pBuffer_tar, &File_Info, sizeof(File_Info) );
        
        write( nFile_out, pBuffer_tar, sizeof(File_Info) );
        memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
    }
#else
    memcpy( pBuffer_tar, &File_Info, sizeof(File_Info) );
    write( nFile_out, pBuffer_tar, sizeof(File_Info) );
    //nWrite_Size += sizeof(File_Info);
    memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) );
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

#if 0 // compress test
            if( Option_z )
            {
                compress(deflate_data, &deflate_size, pBuffer_tar, nWrite_Size );
                //uncompress(pBuffer_tar, &inflate_size, deflate_data, deflate_size );
                write( nFile_out, deflate_data, deflate_size );
                memset( deflate_data, 0x00, sizeof(deflate_data) ); // Init buffer
                deflate_size = DBUF;
            }
            else
            {
                write( nFile_out, pBuffer_tar, nWrite_Size );
            }
#else
            write( nFile_out, pBuffer_tar, nWrite_Size );
#endif
            memset( pBuffer_tar, 0x00, sizeof(pBuffer_tar) ); // Init buffer
            nFile_Size -= nWrite_Size;
            nWrite_Size = 0;
        }
    }            

    //(*nFile_Count)++;         
    close(nFile_arg);   

    return 0;
}

void Create_Archive_Directory(char *pFile_Name, int nFile_out, unsigned short nDirecotry_Depth, char *pPrev_Path)
{
    File_Info File_Info;
    char pBuffer_tar[BUFFER_SIZE] = { 0, };
    memset( &File_Info, 0x00, sizeof(File_Info) );

    File_Info.nKind = KIND_DIRECTORY;
    snprintf( File_Info.pName, sizeof(File_Info.pName), "%s", pFile_Name );
    File_Info.nDirectory_Depth = nDirecotry_Depth;
    snprintf( File_Info.pPrevPath, sizeof(File_Info.pPrevPath), "%s", pPrev_Path );
    memcpy( pBuffer_tar, &File_Info, sizeof(File_Info) );

    // test
    // printf("File_Info.pName : %s\n", File_Info.pName);
    // printf("pBuffer_tar(kind) : %d\n", pBuffer_tar[0]);
    // printf("pBuffer_tar(name) : %s\n", pBuffer_tar+2);

    write( nFile_out, pBuffer_tar, sizeof(File_Info) );
}


int Extract_Archive_Directory(File_Info *File_Info)
{
    DIR *dir_ptr = NULL;
    struct stat sb;
    int nReturn_Value = KIND_FILE;

    // test
    char *test[100] = { 0, };

    static char pTest[100] = { 0, };

    printf("1. File_Info->nDirectory_Depth[%d]\n", File_Info->nDirectory_Depth);

    if( File_Info->nDirectory_Depth == 0)
    {
        mkdir(File_Info->pName, S_IRWXU);
        nReturn_Value = KIND_DIRECTORY;
    }
    else
    {
        printf("File_Info->pPrevPath : %s\n", File_Info->pPrevPath);
        // dir_ptr = opendir( File_Info->pPrevPath);
        // if( dir_ptr == NULL )
        // {
        //     printf("error\n");

        //     return -1;
        // }

        // test
         if(strcmp(pTest, File_Info->pPrevPath))
         {
            // test
            printf("chdir : %s\n", File_Info->pPrevPath);

            chdir(File_Info->pPrevPath);
            snprintf(pTest, sizeof(pTest), "%s", File_Info->pPrevPath);

            // test
            getcwd(test, 100);
            printf("current working directory : %s\n", test);
         }


        if( File_Info->nKind == KIND_DIRECTORY )
        {
            mkdir(File_Info->pName, S_IRWXU);
            nReturn_Value = KIND_DIRECTORY;
        }

        // test
        printf("File_Info->pName : %s\n", File_Info->pName);
        
        // closedir(dir_ptr);
    }

    return nReturn_Value;
}


int Check_Directory(char *pFile_Name, int nFile_out, unsigned short nDirectory_Cnt)
{
    int nReturn_Value = 0;

    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;

    struct stat sb;

    char pFile_Name_Temp[100][100] = { NULL, };
    //static char *pFile_Name_Temp[100] = { NULL, };
    static unsigned short nFile_Type_Temp[100] = { 0, };
    unsigned short nFile_Cnt = 0;
    int nIdx = 0;

    static unsigned short nDirectory_Depth[50] = { 0, };

    char Backup_Working_Directory[100] = { 0, };

    // test
    char *test[100] = { 0, };
    bool bCnt_Up_Flag = false;


    if( (dir_ptr = opendir(pFile_Name)) == NULL )
    {
        printf("error\n");

        return -1;
    }

    //nDirectory_Depth[nDirectory_Cnt]++;

    // test
    printf("nDirectory_Cnt[%d]\n", nDirectory_Cnt);
    
    chdir(pFile_Name);

    printf("chdir : %s\n", pFile_Name);


    while( (file = readdir(dir_ptr)) != NULL )
    {
        if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
        { 
            continue; 
        }

#if 1
        //chdir(pFile_Name);

        nReturn_Value = Check_File_Stat( file->d_name, &sb );
        if( nReturn_Value == -1 )
        {
            // test
            printf("error point file name : %s\n", file->d_name);
            getcwd(test, 100);
            printf("current working direcotry : %s\n", test);
            // test
            
            continue;
        }
#else
        // test
        getcwd(test, 100);
        printf("test_1 : %s\n", test);

        // strcat(test, "/");
        // strcat(test, file->d_name);
        // printf("after test : %s\n", test);

        chdir(pFile_Name);

        getcwd(test, 100);
        printf("test_2 : %s\n", test);

        //if( stat(pFile_Name, &sb) == -1 )
        //if( lstat("./test/test_1", &sb) == -1 )
        if( lstat(file->d_name, &sb) == -1 )
        //if( stat(file->d_name, &sb) == -1 )
        {
            perror("fstat error_function\n");
            continue;
        }
#endif        
        // test
        //printf("nDirectory_Cnt[%d]\n", nDirectory_Cnt);

        if( Check_File_Type( sb ) )
        //if( S_ISDIR(sb.st_mode) )
        { // directory
            Create_Archive_Directory( file->d_name, nFile_out, nDirectory_Cnt, pFile_Name );

            //Check_Directory( file->d_name, nFile_out, nDirectory_Depth[nDirectory_Cnt] );

            //printf("nDirectory_Depth_Directory[%d]\n", nDirectory_Cnt);

            nFile_Type_Temp[nFile_Cnt] = KIND_DIRECTORY;

            // test
            getcwd(test, 100);
            printf("current working direcotry : %s\n", test);
        }
        else // file
        {
            printf("file name : %s\n", file->d_name);

            Create_Archive( file->d_name, nFile_out, false, nDirectory_Cnt, pFile_Name);
            printf("nDirectory_Depth_File[%d]\n", nDirectory_Cnt);

            // test
            getcwd(test, 100);
            printf("current working direcotry : %s\n", test);
        }

        snprintf(pFile_Name_Temp[nFile_Cnt], 100, "%s", file->d_name);
        //memcpy( pFile_Name_Temp[nFile_Cnt], file->d_name, 100 );

        printf("pFile_Name_Temp[%d] = %s\n", nFile_Cnt, pFile_Name_Temp[nFile_Cnt]);

        nFile_Cnt++;
    }

    bCnt_Up_Flag = true;

    for( nIdx = 0; nIdx < nFile_Cnt; nIdx++ )
    {
        if( nFile_Type_Temp[nIdx] == KIND_DIRECTORY )
        {
            if( bCnt_Up_Flag )
            {
                nDirectory_Cnt++;
            }
            getcwd(Backup_Working_Directory, 100);
            printf("Backup_Working_Directory : %s\n", Backup_Working_Directory);
            //Check_Directory( pFile_Name_Temp[nIdx], nFile_out, nDirectory_Depth[nDirectory_Cnt] );
            Check_Directory( pFile_Name_Temp[nIdx], nFile_out, nDirectory_Cnt );
            chdir(Backup_Working_Directory);

            bCnt_Up_Flag = false;
        }

    }

    closedir(dir_ptr);

    return 0;
}


int Check_File_Stat(char *pFile_Name, struct stat *sb)
{
    //struct stat sb;

    if( lstat(pFile_Name, sb) == -1 )
    {
        perror("fstat error_function_Check_File_Stat\n");
        return -1;
    }

    return 0;
}

/*  Return Value
    0 : File
    1 : Directory   */
int Check_File_Type(struct stat sb)
{
    int nReturn = 0;

#if 1
    if( S_ISDIR(sb.st_mode) )
    {
        nReturn = 1;
    }
    else
    {
        nReturn = 0;
    }
#else
    if((sb.st_mode & (S_IFMT)) == S_IFDIR)
    {
        nReturn = 1;
    }
    else
    {
        nReturn = 0;
    }
#endif    

    return nReturn;
}