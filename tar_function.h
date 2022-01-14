
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <zlib.h>

// mkdir test
#include <dirent.h>


#ifndef TAR_FUNCTION
#define TAR_FUNCTION


#define BUFFER_SIZE     (1024*5)
#define READ_SIZE       (128)

#define TAR_OPTION_Z    (1 << 0)
#define TAR_OPTION_X    (1 << 1)
#define TAR_OPTION_FLAG (1 << 2)
#define TAR_OPTION_C    (1 << 3)
#define TAR_OPTION_N    (1 << 4)

#define OPTION_C_DEFAULT_FILE_NAME  ("Option_c_test")
#define OPTION_Z_DEFAULT_FILE_NAME  ("Option_z_test")

#if 1 // compress, uncompress test
#define DBUF  (BUFFER_SIZE)
#define BUF   (DBUF*2)
#endif    

#define KIND_FILE           (0)
#define KIND_DIRECTORY      (1)


typedef struct 
{
    /* data */
    unsigned short nKind;
    char pName[100];
    unsigned int nSize; // file
    //char pPath[128];

    unsigned short nDirectory_Depth;
    char pPrevPath[100];

    // struct
    // {
        
    // }Directory_Info[10];
    
}File_Info;


/*  Function  */
int Get_Option(int argc, char **argv);
int Create_Archive(char *pFile_Name, int nFile_out, bool Option_z, unsigned short nDirecotry_Depth, char *pPrev_Path);
void Create_Archive_Directory(char *pFile_Name, int nFile_out, unsigned short nDirecotry_Depth, char *pPrev_Path);

int Extract_Archive_Directory(File_Info *File_Info);

int Check_Directory(char *pFile_Name, int nFile_out, unsigned short nDirectory_Cnt);

int Check_File_Stat(char *pFile_Name, struct stat *sb);

int Check_File_Type(struct stat sb);

#endif