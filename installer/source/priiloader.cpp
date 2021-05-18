#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/machine/asm.h>
#include <ogc/isfs.h>
#include <ogc/ios.h>
#include <ogc/usbgecko.h>
#include <wiiuse/wpad.h>

#include "debug.h"
#include "sha1.h"
#include "haxx_certs.h"
#include "su_tmd.h"
#include "su_tik.h"
#include "priiloader_app.h"
#include "bootloader_dol.h"

#include "system.h"

#define TITLE_UPPER(x) (u32)(x >> 32)
#define TITLE_LOWER(x) (u32)(x & 0xFFFFFFFF)
#define ALIGN32(x) (((x) + 31) & ~31)

char original_app[ISFS_MAXPATH];
char copy_app[ISFS_MAXPATH];

char TMD_Path[ISFS_MAXPATH];
char TMD_Path2[ISFS_MAXPATH];

u32 tmd_size;
static u8 tmd_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
static signed_blob *mTMD;
static tmd *rTMD;
static u64 title_id = 0x0000000100000002LL;
//static u64 title_id = 0x0001000147534654LL;
typedef struct
{
    u32 owner;
    u16 group;
    u8 attributes;
    u8 ownerperm;
    u8 groupperm;
    u8 otherperm;
} ATTRIBUTE_PACKED Nand_Permissions;

typedef struct {
    u8 autoboot;
    u32 version;
    u8 ReturnTo;
    u8 ShutdownTo;
    u8 StopDisc;
    u8 LidSlotOnError;
    u8 IgnoreShutDownMode;
    u32 BetaVersion;
    u8 SystemMenuIOS;
    u8 UseSystemMenuIOS;
    u8 BlackBackground;
    u8 DumpGeckoText;
    u8 PasscheckPriiloader;
    u8 PasscheckMenu;
    u8 ShowBetaUpdates;
} ATTRIBUTE_ALIGN(32) Settings;

enum {
        SETTING_AUTBOOT,
        SETTING_RETURNTO,
        SETTING_SHUTDOWNTO,
        SETTING_STOPDISC,
        SETTING_LIDSLOTONERROR,
        SETTING_IGNORESHUTDOWNMODE,
        SETTING_BETAVERSION,
        SETTING_SYSTEMMENUIOS,
        SETTING_USESYSTEMMENUIOS,
        SETTING_BLACKBACKGROUND,
        SETTING_DUMPGECKOTEXT,
        SETTING_PASSCHECKPRII,
        SETTING_PASSCHECKMENU,
        SETTING_SHOWBETAUPDATES,
};

enum {
        AUTOBOOT_DISABLED,
        AUTOBOOT_HBC,
        AUTOBOOT_BOOTMII_IOS,
        AUTOBOOT_SYS,
        AUTOBOOT_FILE,
        AUTOBOOT_ERROR
};
enum {
        RETURNTO_SYSMENU,
        RETURNTO_PRIILOADER,
        RETURNTO_AUTOBOOT
};

enum {
    SHUTDOWNTO_NONE,
    SHUTDOWNTO_PRIILOADER,
    SHUTDOWNTO_AUTOBOOT
};

void sleepx (u32 seconds) {
    u64 now = gettime();
    while(ticks_to_secs(gettime() - now) < seconds) {
        VIDEO_WaitVSync();
    }
    return;
}

void abort(const char* msg, ...)
{
    va_list args;
    char text[4096];
    va_start( args, msg );
    strcpy( text + vsnprintf( text,4095,msg,args ),"");
    va_end( args );
    printf("\x1b[%u;%dm", 36, 1);
    gprintf("%s, aborting mission...\n", text);
    printf("exitting...\n");
    printf("\x1b[%u;%dm", 37, 1);
    VIDEO_WaitVSync();

    IOS_ReloadIOS(58);

    exit(0);
}

s8 HaveNandPermissions( void ) {
    gprintf("testing permissions...");
    s32 temp = ISFS_Open("/title/00000001/00000002/content/title.tmd",ISFS_OPEN_RW);
    if ( temp < 0 )
    {
        gprintf("no permissions.error %d\n",temp);
        return false;
    }
    else
    {
        ISFS_Close(temp);
        gprintf("and bingo was his name-O\n");
        return true;
    }
}

bool CompareSha1Hash(u8 *Data1,u32 Data1_Size,u8 *Data2,u32 Data2_Size)
{
    if(Data1 == NULL || Data2 == NULL )
    {
        gprintf("Data1 or Data2 == NULL\n");
        return false;
    }
    if(Data1_Size <= 0 || Data2_Size <= 0)
    {
        gprintf("Data1 or Data2 size == NULL\n");
        return false;
    }
    sha1 FileHash_D1;
    sha1 FileHash_D2;
    memset(FileHash_D2, 0xFF, 0x14); // set FileHash_D2 to different value so that if something goes wrong; the check fails
    SHA1(Data1, Data1_Size, FileHash_D1);

    SHA1(Data2, Data2_Size, FileHash_D2);

    if (!memcmp(FileHash_D1,FileHash_D2,sizeof(FileHash_D1)))
    {
        return true;
    }
    else
    {
        gprintf("SHA1 check failed!\n");
    }
    return false;
}
s32 nand_copy(const char *destination,u8* Buf_To_Write_to_Copy, u32 buf_size,Nand_Permissions src_perm)
{
    if( Buf_To_Write_to_Copy == NULL || buf_size < 1 )
    {
        return -1;
    }
    s32 ret, dest_handler;
    gprintf("owner %d group %d attributes %X perm:%X-%X-%X\n", src_perm.owner, (u32)src_perm.group, (u32)src_perm.attributes, (u32)src_perm.ownerperm, (u32)src_perm.groupperm, (u32)src_perm.otherperm);

    //extract filename from destination
    char temp_dest[ISFS_MAXPATH];
    memset(temp_dest,0,ISFS_MAXPATH);
    char *ptemp = NULL;
    ptemp = strstr(destination,"/");
    while(ptemp != NULL && strstr(ptemp+1,"/") != NULL)
    {
        ptemp = strstr(ptemp+1,"/");
    }
    if(ptemp[0] == '/')
    {
        ptemp = ptemp+1;
    }

    //create temp path
    memset(temp_dest,0,ISFS_MAXPATH);
    sprintf(temp_dest,"/tmp/%s",ptemp);
    ISFS_Delete(temp_dest);

    //and go for it
    ret = ISFS_CreateFile(temp_dest,src_perm.attributes,src_perm.ownerperm,src_perm.groupperm,src_perm.otherperm);
    if (ret != ISFS_OK)
    {
        printf("Failed to create file %s. ret = %d\n",temp_dest,ret);
        gprintf("Failed to create file %s. ret = %d\n",temp_dest,ret);
        return ret;
    }
    dest_handler = ISFS_Open(temp_dest,ISFS_OPEN_RW);
    if (dest_handler < 0)
    {
        gprintf("failed to open destination : %s\n",temp_dest);
        ISFS_Delete(temp_dest);
        return dest_handler;
    }

    ret = ISFS_Write(dest_handler,Buf_To_Write_to_Copy,buf_size);
    if (ret < 0)
    {
        gprintf("failed to write destination : %s\n",temp_dest);
        ISFS_Close(dest_handler);
        ISFS_Delete(temp_dest);
        return ret;
    }
    ISFS_Close(dest_handler);
    s32 temp = 0;
    u8 *Data2 = NULL;
    STACK_ALIGN(fstats,D2stat,sizeof(fstats),32);
    /*if (D2stat == NULL)
    {
        temp = -1;
        goto free_and_Return;
    }*/
    dest_handler = ISFS_Open(temp_dest,ISFS_OPEN_RW);
    if(dest_handler < 0)
    {
        gprintf("temp_dest open error %d\n",dest_handler);
        temp = -2;
        goto free_and_Return;
    }
    temp = ISFS_GetFileStats(dest_handler,D2stat);
    if(temp < 0)
    {
        goto free_and_Return;
    }
    Data2 = (u8*)memalign(32,ALIGN32(D2stat->file_length));
    if (Data2 == NULL)
    {
        temp = -3;
        goto free_and_Return;
    }
    if( ISFS_Read(dest_handler,Data2,D2stat->file_length) > 0 )
    {
            if( !CompareSha1Hash(Buf_To_Write_to_Copy,buf_size,Data2,D2stat->file_length))
            {
                temp = -4;
                goto free_and_Return;
            }
    }
    else
    {
        temp = -5;
        goto free_and_Return;
    }
    if(Data2)
    {
        free(Data2);
        Data2 = NULL;
    }
    ISFS_Close(dest_handler);
    //so it was written to /tmp correctly. lets call ISFS_Rename and compare AGAIN
    ISFS_Delete(destination);
    ret = ISFS_Rename(temp_dest,destination);
    if(ret < 0 )
    {
        gprintf("nand_copy(buf) : rename returned %d\n",ret);
        temp = -6;
        goto free_and_Return;
    }
free_and_Return:
    if(Data2 != NULL)
    {
        free(Data2);
        Data2 = NULL;
    }
    ISFS_Close(dest_handler);
    if (temp < 0)
    {
        gprintf("temp %d\n",temp);
        //ISFS_Delete(destination);
        return -80;
    }
    return 1;
}
s32 nand_copy(const char *source, const char *destination,Nand_Permissions src_perm)
{
    //variables
    u8 *buffer = NULL;
    STACK_ALIGN(fstats,status,sizeof(fstats),32);
    s32 file_handler, ret;
    sha1 FileHash_D1;
    memset(FileHash_D1, 0, 0x14);
    sha1 FileHash_D2;
    memset(FileHash_D2, 0xFF, 0x14); //place different data in D2 so that if something goes wrong later on, the comparison will fail

    //variables - temp dir & SHA1 check
    char temp_dest[ISFS_MAXPATH];
    memset(temp_dest,0,ISFS_MAXPATH);
    char *ptemp = NULL;
    u8 temp = 0;

    //get temp filename
    ptemp = strstr(destination,"/");
    while(ptemp != NULL && strstr(ptemp+1,"/") != NULL)
    {
        ptemp = strstr(ptemp+1,"/");
    }
    if(ptemp[0] == '/')
    {
        ptemp = ptemp+1;
    }
    memset(temp_dest,0,ISFS_MAXPATH);
    sprintf(temp_dest,"/tmp/%s",ptemp);

    //get data into pointer from original file
    file_handler = ISFS_Open(source,ISFS_OPEN_READ);
    if (file_handler < 0)
    {
        gprintf("failed to open source : %s\n",source);
        return file_handler;
    }

    ret = ISFS_GetFileStats(file_handler,status);
    if (ret < 0)
    {
        printf("\n\nFailed to get information about %s!\n",source);
        sleepx(2);
        ISFS_Close(file_handler);
        return ret;
    }

    buffer = (u8 *)memalign(32,ALIGN32(status->file_length));
    if (buffer == NULL)
    {
        gprintf("buffer failed to align\n");
        sleepx(2);
        ISFS_Close(file_handler);
        return 0;
    }
    memset(buffer,0,status->file_length);
    ret = ISFS_Read(file_handler,buffer,status->file_length);
    if (ret < 0)
    {
        printf("\n\nFailed to Read Data from %s!\n",source);
        sleepx(2);
        ISFS_Close(file_handler);
        free(buffer);
        buffer = NULL;
        return ret;
    }
    ISFS_Close(file_handler);
    //everything read into buffer. generate SHA1 hash of the buffer
    SHA1(buffer, status->file_length, FileHash_D1);
    //done, lets create temp file and write :')

    ISFS_Delete(temp_dest);
    ISFS_CreateFile(temp_dest,src_perm.attributes,src_perm.ownerperm,src_perm.groupperm,src_perm.otherperm);
    //created. opening it...
    file_handler = ISFS_Open(temp_dest,ISFS_OPEN_RW);
    if (file_handler < 0)
    {
        gprintf("failed to open destination : %s\n",temp_dest);
        ISFS_Delete(temp_dest);
        free(buffer);
        buffer = NULL;
        return file_handler;
    }
    ret = ISFS_Write(file_handler,buffer,status->file_length);
    if (ret < 0)
    {
        gprintf("failed to write destination : %s\n",destination);
        ISFS_Close(file_handler);
        ISFS_Delete(temp_dest);
        free(buffer);
        buffer = NULL;
        return ret;
    }
    //write done. reopen file for reading and compare SHA1 hash
    ISFS_Close(file_handler);
    free(buffer);
    buffer = NULL;
    memset(status,0,sizeof(fstats));
    file_handler = ISFS_Open(temp_dest,ISFS_OPEN_READ);
    if(!file_handler)
    {
        temp = -1;
        goto free_and_Return;
    }
    ret = ISFS_GetFileStats(file_handler,status);
    if (ret < 0)
    {
        ISFS_Close(file_handler);
        temp = -2;
        goto free_and_Return;
    }
    buffer = (u8 *)memalign(32,ALIGN32(status->file_length));
    if (buffer == NULL)
    {
        gprintf("buffer failed to align\n");
        ISFS_Close(file_handler);
        temp = -3;
        goto free_and_Return;
    }
    memset(buffer,0,status->file_length);
    if( ISFS_Read(file_handler,buffer,status->file_length) < 0 )
    {
        temp = -4;
        goto free_and_Return;
    }
    ISFS_Close(file_handler);
    SHA1(buffer, status->file_length, FileHash_D2);
    free(buffer);
    buffer = NULL;

    if (!memcmp(FileHash_D1, FileHash_D2, sizeof(FileHash_D1)))
    {
        gprintf("nand_copy : SHA1 hash success\n");
        ISFS_Delete(destination);
        ret = ISFS_Rename(temp_dest,destination);
        gprintf("ISFS_Rename ret %d\n",ret);
        if ( ret < 0)
            temp = -5;
        goto free_and_Return;
    }
    else
    {
        temp = -6;
        goto free_and_Return;
    }
free_and_Return:
    if(buffer)
    {
        free(buffer);
        buffer = NULL;
    }
    if (temp < 0)
    {
        gprintf("nand_copy temp %d fail o.o;\n",temp);
        ISFS_Delete(temp_dest);
        return -80;
    }
    return 1;
}
void proccess_delete_ret( s32 ret )
{
    if(ret == -106)
    {
        printf("\x1b[%u;%dm", 32, 1);
        printf("Not found\n");
        printf("\x1b[%u;%dm", 37, 1);
    }
    else if(ret == -102)
    {
        printf("\x1b[%u;%dm", 33, 1);
        printf("Error deleting file: access denied\n");
        printf("\x1b[%u;%dm", 37, 1);
    }
    else if (ret < 0)
    {
        printf("\x1b[%u;%dm", 33, 1);
        printf("Error deleting file. error %d\n",ret);
        printf("\x1b[%u;%dm", 37, 1);
    }
    else
    {
        printf("\x1b[%u;%dm", 32, 1);
        printf("Deleted\n");
        printf("\x1b[%u;%dm", 37, 1);
    }
    return;
}
void Delete_Priiloader_Files( u8 mode )
{
    bool settings = false;
    bool hacks = false;
    bool password = false;
    bool main_bin = false;
    bool ticket = false;
    switch(mode)
    {
        case 2: //remove
            main_bin = true;
            ticket = true;
        case 0: //install
            hacks = true;
        case 1: //update
            settings = true;
            password = true;
        default:
            break;
    }
    s32 ret = 0;
    static char file_path[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
    memset(file_path,0,ISFS_MAXPATH);
    if(password)
    {
        sprintf(file_path, "/title/%08x/%08x/data/password.txt",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("password.txt : %d\n",ret);
        printf("password file : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);
    }
    if(settings)
    {
        sprintf(file_path, "/title/%08x/%08x/data/loader.ini",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("loader.ini : %d\n",ret);
        printf("Settings file : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);
    }
    //its best we delete that ticket but its completely useless and will only get in our
    //way when installing again later...
    if(ticket)
    {
        sprintf(file_path, "/title/%08x/%08x/content/ticket",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("ticket : %d\n",ret);
        printf("Ticket : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);
    }
    if(hacks)
    {
        sprintf(file_path, "/title/%08x/%08x/data/hacks_s.ini",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("hacks_s.ini : %d\n",ret);
        printf("Hacks_s.ini : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);

        sprintf(file_path, "/title/%08x/%08x/data/hacks.ini",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("hacks.ini : %d\n",ret);
        printf("Hacks.ini : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);

        sprintf(file_path, "/title/%08x/%08x/data/hacksh_s.ini",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("hacksh_s.ini : %d\n",ret);
        printf("Hacksh_s.ini : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);

        sprintf(file_path, "/title/%08x/%08x/data/hackshas.ini",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("hacks_hash : %d\n",ret);
        printf("Hacks_hash : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);

    }
    if(main_bin)
    {
        sprintf(file_path, "/title/%08x/%08x/data/main.nfo",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("main.nfo : %d\n",ret);
        sprintf(file_path, "/title/%08x/%08x/data/main.bin",(u32)(title_id >> 32),(u32) (title_id & 0xFFFFFFFF));
        ret = ISFS_Delete(file_path);
        gprintf("main.bin : %d\n",ret);
        printf("main.bin : ");
        proccess_delete_ret(ret);
        memset(file_path,0,ISFS_MAXPATH);
    }
    return;
}
s8 PatchTMD( u8 delete_mode )
{
    Nand_Permissions Perm;
    memset(&Perm,0,sizeof(Nand_Permissions));
    u8 SaveTmd = 0;

    sha1 FileHash;
    u8 *TMD_chk = NULL;
    s32 fd = 0;
    s32 r = 0;
#ifdef _DEBUG
    gprintf("Path : %s\n",TMD_Path);
#endif
    r = ISFS_GetAttr(TMD_Path, &Perm.owner, &Perm.group, &Perm.attributes, &Perm.ownerperm, &Perm.groupperm, &Perm.otherperm);
    if(r < 0 )
    {
        //attribute getting failed. returning to default
        printf("\x1b[%u;%dm", 33, 1);
        printf("\nWARNING : failed to get file's permissions. using defaults\n");
        printf("\x1b[%u;%dm", 37, 1);
        gprintf("permission failure on desination! error %d\n",r);
        gprintf("writing with max permissions\n");
        Perm.ownerperm = 3;
        Perm.groupperm = 3;
        Perm.otherperm = 0;
    }
    else
    {
        gprintf("r %d owner %d group %d attributes %X perm:%X-%X-%X\n", r, Perm.owner, Perm.group, Perm.attributes, Perm.ownerperm, Perm.groupperm, Perm.otherperm);
    }
    if(delete_mode == 0)
    {
        gprintf("patching TMD...\n");
        printf("Patching TMD...");

    }
    else
    {
        //return 1;
        gprintf("restoring TMD...\n");
        printf("Restoring System Menu TMD...\n");
    }

    fd = ISFS_Open(TMD_Path2,ISFS_OPEN_READ);
    if(fd < 0)
    {
        if(delete_mode)
        {
            printf("TMD backup not found. leaving TMD alone...\n");
            return 1;
        }
        else
        {
            //got to make tmd copy :)
            gprintf("Making tmd backup...\n");
            r = nand_copy(TMD_Path2,tmd_buf,tmd_size,Perm);
            if ( r < 0)
            {
                gprintf("Failure making TMD backup.error %d\n",r);
                printf("TMD backup/Patching Failure : error %d",r);
                goto _return;
            }
        }
        fd = 0;
    }
    ISFS_Close(fd);
    gprintf("TMD backup found\n");
    //not so sure why we'd want to delete the tmd modification but ok...
    if(delete_mode)
    {
        if ( nand_copy(TMD_Path2,TMD_Path,Perm) < 0)
        {
            if(r == -80)
            {
                nand_copy(TMD_Path,TMD_Path2,Perm);
                abort("TMD restoring failure.");
            }
            else
            {
                printf("\x1b[%u;%dm", 33, 1);
                printf("UNABLE TO RESTORE THE SYSTEM MENU TMD!!!\n\nTHIS COULD BRICK THE WII SO PLEASE REINSTALL SYSTEM MENU\nWHEN RETURNING TO THE HOMEBREW CHANNEL!!!\n\n");
                printf("\x1b[%u;%dm", 37, 1);
                nand_copy(TMD_Path,TMD_Path2,Perm);
                exit(0);
            }
        }
        else
            ISFS_Delete(TMD_Path2);
        return 1;
    }
    else
    {
        //check if version is the same
        STACK_ALIGN(fstats,TMDb_status,sizeof(fstats),32);
        static u8 tmdb_buf[MAX_SIGNED_TMD_SIZE] ATTRIBUTE_ALIGN(32);
        static signed_blob *mbTMD;
        static tmd* pbTMD;

        fd = ISFS_Open(TMD_Path2,ISFS_OPEN_READ);
        if (fd < 0)
        {
            gprintf("TMD bCheck : failed to open source : %s\n",TMD_Path2);
            goto patch_tmd;
        }
        r = ISFS_GetFileStats(fd,TMDb_status);
        if (r < 0)
        {
            gprintf("TMD bCheck : Failed to get information about %s!\n",TMD_Path2);
            ISFS_Close(fd);
            goto patch_tmd;
        }
        memset(tmdb_buf,0,MAX_SIGNED_TMD_SIZE);

        r = ISFS_Read(fd,tmdb_buf,TMDb_status->file_length);
        if (r < 0)
        {
            gprintf("TMD bCheck : Failed to Read Data from %s!\n",TMD_Path2);
            ISFS_Close(fd);
            goto patch_tmd;
        }
        ISFS_Close(fd);
        mbTMD = (signed_blob *)tmdb_buf;
        pbTMD = (tmd*)SIGNATURE_PAYLOAD(mbTMD);
        if (pbTMD->title_version != rTMD->title_version)
        {
            gprintf("TMD bCheck : backup TMD version mismatch: %d & %d\n",rTMD->title_version,pbTMD->title_version);
            //got to make tmd copy :)
            r = nand_copy(TMD_Path2,tmd_buf,tmd_size,Perm);
            if ( r < 0)
            {
                gprintf("TMD bCheck : Failure making TMD backup.error %d\n",r);
                printf("TMD backup/Patching Failure : error %d",r);
                goto _return;
            }
        }
        else
            gprintf("TMD bCheck : backup TMD is correct\n");
        r = 0;
    }
patch_tmd:
    gprintf("detected access rights : 0x%08X\n",rTMD->access_rights);
    if(rTMD->access_rights == 0x03)
    {
        gprintf("no AHBPROT modification needed\n");
    }
    else
    {
        rTMD->access_rights = 0x03;
        DCFlushRange(rTMD,sizeof(tmd));
        if(rTMD->access_rights != 0x03)
        {
            gprintf("rights change failure.\n");
            goto _return;
        }
        SaveTmd++;
    }
    gprintf("checking Boot app SHA1 hash...\n");
    gprintf("bootapp ( %d ) SHA1 hash = %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",rTMD->boot_index
        ,rTMD->contents[rTMD->boot_index].hash[0],rTMD->contents[rTMD->boot_index].hash[1],rTMD->contents[rTMD->boot_index].hash[2],rTMD->contents[rTMD->boot_index].hash[3]
        ,rTMD->contents[rTMD->boot_index].hash[4],rTMD->contents[rTMD->boot_index].hash[5],rTMD->contents[rTMD->boot_index].hash[6],rTMD->contents[rTMD->boot_index].hash[7]
        ,rTMD->contents[rTMD->boot_index].hash[8],rTMD->contents[rTMD->boot_index].hash[9],rTMD->contents[rTMD->boot_index].hash[10],rTMD->contents[rTMD->boot_index].hash[11]
        ,rTMD->contents[rTMD->boot_index].hash[12],rTMD->contents[rTMD->boot_index].hash[13],rTMD->contents[rTMD->boot_index].hash[14],rTMD->contents[rTMD->boot_index].hash[15]
        ,rTMD->contents[rTMD->boot_index].hash[16],rTMD->contents[rTMD->boot_index].hash[17],rTMD->contents[rTMD->boot_index].hash[18],rTMD->contents[rTMD->boot_index].hash[19]);
    gprintf("generated priiloader SHA1 : ");
    SHA1((u8*)priiloader_app, priiloader_app_size, FileHash);

    if (!memcmp(rTMD->contents[rTMD->boot_index].hash, FileHash, sizeof(FileHash) ) )
    {
        gprintf("no SHA hash change needed\n");
    }
    else
    {
        memcpy(rTMD->contents[rTMD->boot_index].hash,FileHash,sizeof(FileHash));
        gprintf("%08x %08x %08x %08x %08x\n",FileHash[0],FileHash[1],FileHash[2],FileHash[3],FileHash[4]);
        DCFlushRange(rTMD,sizeof(tmd));
        SaveTmd++;
    }
    if(SaveTmd > 0)
    {
        gprintf("saving TMD\n");
        r = nand_copy(TMD_Path,tmd_buf,tmd_size,Perm);
        if(r < 0 )
        {
            gprintf("nand_copy failure. error %d\n",r);
            if(r == -80)
                goto _checkreturn;
            else
                goto _return;
        }
    }
    else
    {
        gprintf("no TMD mod's needed\n");
        printf("no TMD modifications needed\n");
        goto _return;
    }
    printf("Done\n");
_return:
    if (fd < r)
    {
        r = fd;
    }
    if(fd)
    {
        ISFS_Close(fd);
    }
    if (r < 0)
    {
        printf("\x1b[%u;%dm", 33, 1);
        printf("\nWARNING!!\nInstaller couldn't Patch the system menu TMD.\n");
        fd = ISFS_Open(TMD_Path2,ISFS_OPEN_RW);
        if(fd >= 0)
        {
            //the backup is there. as safety lets copy it back.
            ISFS_Close(fd);
            nand_copy(TMD_Path2,TMD_Path,Perm);
        }
        abort("TMD failure");
        return -1;
    }
    else
        return 1;
_checkreturn:
    if(fd)
    {
        ISFS_Close(fd);
    }
    if(TMD_chk)
    {
        free(TMD_chk);
        TMD_chk = NULL;
    }
    printf("\x1b[%u;%dm", 33, 1);
    printf("\nWARNING!!\n  Installer could not calculate the Checksum for the TMD!");
    printf("\x1b[%u;%dm", 37, 1);
    printf("reverting changes...\n");
    nand_copy(TMD_Path2,TMD_Path,Perm);
    abort("TMD Patch failure\n");
    return -80;
}
s8 CopyTicket ( )
{
    s32 fd = 0;
    char TIK_Path_dest[64];
    char TIK_Path_org[64];
    memset(TIK_Path_dest,0,64);
    memset(TIK_Path_org,0,64);
    sprintf(TIK_Path_dest, "/title/%08x/%08x/content/ticket",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
    sprintf(TIK_Path_org, "/ticket/%08x/%08x.tik",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
    gprintf("Checking for copy ticket...\n");
    fd = ISFS_Open(TIK_Path_dest,ISFS_OPEN_READ);
    if (fd >= 0)
    {
        ISFS_Close(fd);
        printf("Skipping copy of system menu ticket...\n");
        return 1;
    }
    switch(fd)
    {
        case ISFS_EINVAL:
            abort("Unable to read ticket.path is wrong/too long or ISFS isn't init yet?");
            break;
        case ISFS_ENOMEM:
            abort("Unable to read ticket.(Out of memory)");
            break;
        case -102:
            abort("Unauthorised to get ticket. is ios%d trucha signed?",IOS_GetVersion());
            break;
        default:
            if(fd < 0)
                abort("Unable to read ticket. error %d. ",fd);
        case -106:
            printf("Priiloader system menu ticket not found.\n\tTrying to read original ticket...\n");
            break;
    }
    fd = ISFS_Open(TIK_Path_org,ISFS_OPEN_READ);
    //"/ticket/00000001/00000002.tik" -> original path which should be there on every wii.
    if (fd < 0)
    {
        switch(fd)
        {
            case ISFS_EINVAL:
                abort("Unable to read ticket.path is wrong/too long or ISFS isn't init yet?");
                break;
            case ISFS_ENOMEM:
                abort("Unable to read ticket.(Out of memory)");
                break;
            case -106:
                abort("Ticket not found");
                break;
            case -102:
                abort("Unauthorised to get ticket. is ios%d trucha signed?",IOS_GetVersion());
                break;
            default:
                abort("Unable to read ticket. error %d. ",fd);
                break;
        }

    }
    ISFS_Close(fd);
    printf("Copying system menu ticket...");
    Nand_Permissions TikPerm;
    memset(&TikPerm,0,sizeof(Nand_Permissions));
    TikPerm.otherperm = 3;
    TikPerm.groupperm = 3;
    TikPerm.ownerperm = 3;
    if (nand_copy(TIK_Path_org,TIK_Path_dest,TikPerm) < 0)
    {
        abort("Unable to copy the system menu ticket");
    }
    printf("Done!\n");
    return 1;
}

bool CheckForPriiloader( void )
{
    bool ret = false;
    s32 fd = 0;
    printf("Checking for Priiloader...\n");
    gprintf("checking for SystemMenu Dol\n");
    fd = ISFS_Open(copy_app,ISFS_OPEN_RW);
    if (fd < 0)
    {
        printf("Priiloader not found : Installing Priiloader...\n\n");
        ret = false;
    }
    else
    {
        ISFS_Close(fd);
        printf("Priiloader installation found : Updating Priiloader...\n\n");
        ret = true;
    }
    return ret;
}

s8 WritePriiloader( bool priiloader_found )
{
    s32 ret = 0;
    s32 fd = 0;
    Nand_Permissions SysPerm;
    if(priiloader_found == false)
    {
        memset(&SysPerm,0,sizeof(Nand_Permissions));
        SysPerm.otherperm = 3;
        SysPerm.groupperm = 3;
        SysPerm.ownerperm = 3;
        //system menu coping
        printf("Moving System Menu app...");
        ret = nand_copy(original_app,copy_app,SysPerm);
        if (ret < 0)
        {
            if (ret == -80)
            {
                //checksum issues
                printf("\x1b[%u;%dm", 33, 1);
                printf("\nWARNING!!\n  Installer could not calculate the Checksum for the System menu app");

                printf("reverting changes...\n");
                ISFS_Delete(copy_app);
                abort("System Menu Copying Failure");
            }
            else
                abort("\nUnable to move the system menu. error %d",ret);
        }
        else
        {
            gprintf("Moving System Menu Done\n");
            printf("Done!\n");
        }
    }
    ret = 0;
    //sys_menu app moved. lets write priiloader
    STACK_ALIGN(fstats,status,sizeof(fstats),32);
    memset(&SysPerm,0,sizeof(Nand_Permissions));
    SysPerm.otherperm = 3;
    SysPerm.groupperm = 3;
    SysPerm.ownerperm = 3;

    printf("Writing Priiloader app...");
    gprintf("Writing Priiloader\n");

    char temp_dest[ISFS_MAXPATH];
    memset(temp_dest,0,ISFS_MAXPATH);
    char *ptemp = NULL;
    ptemp = strstr(original_app,"/");
    while(ptemp != NULL && strstr(ptemp+1,"/") != NULL)
    {
        ptemp = strstr(ptemp+1,"/");
    }
    if(ptemp[0] == '/')
    {
        ptemp = ptemp+1;
    }
    memset(temp_dest,0,ISFS_MAXPATH);
    sprintf(temp_dest,"/tmp/%s",ptemp);
    ISFS_Delete(temp_dest);
    ret = ISFS_CreateFile(temp_dest,SysPerm.attributes,SysPerm.ownerperm,SysPerm.groupperm,SysPerm.otherperm);

    fd = ISFS_Open(temp_dest,ISFS_OPEN_RW);
    if (fd < 0)
    {
        gprintf("error %d\n",fd);
        abort("\nFailed to open file for Priiloader writing");
    }
    ret = ISFS_Write(fd,priiloader_app,priiloader_app_size);
    if (ret < 0 ) //check if the app was writen correctly
    {
        ISFS_Close(fd);
        ISFS_Delete(copy_app);
        ISFS_Delete(temp_dest);
        gprintf("Write failed. ret %d\n",ret);
        abort("\nWrite of Priiloader app failed");
    }
    ISFS_Close(fd);

    //SHA1 check here
    fd = ISFS_Open(temp_dest,ISFS_OPEN_READ);
    if (fd < 0)
    {
        ISFS_Delete(copy_app);
        abort("\nFailed to open file for Priiloader checking");
    }
    if (ISFS_GetFileStats(fd,status) < 0)
    {
        ISFS_Close(fd);
        ISFS_Delete(copy_app);
        abort("Failed to get stats of %s. System Menu Recovered",temp_dest);
    }
    else
    {
        if ( status->file_length != priiloader_app_size )
        {
            ISFS_Close(fd);
            ISFS_Delete(copy_app);
            abort("Written Priiloader app isn't the correct size.System Menu Recovered");
        }
        else
        {
            gprintf("Size Check Success\n");
            printf("Size Check Success!\n");
        }
    }
    u8 *AppData = (u8 *)memalign(32,ALIGN32(status->file_length));
    if (AppData)
        ret = ISFS_Read(fd,AppData,status->file_length);
    else
    {
        ISFS_Close(fd);
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure! MemAlign Failure of AppData\n");
    }
    ISFS_Close(fd);
    if (ret < 0)
    {
        if (AppData)
        {
            free(AppData);
            AppData = NULL;
        }
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure! read of priiloader app returned %u\n",ret);
    }
    if(CompareSha1Hash((u8*)priiloader_app,priiloader_app_size,AppData,status->file_length))
        printf("Checksum comparison Success!\n");
    else
    {
        if (AppData)
        {
            free(AppData);
            AppData = NULL;
        }
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure!\n");
    }
    if (AppData)
    {
        free(AppData);
        AppData = NULL;
    }
    // rename and do a final SHA1 chezck
    ISFS_Delete(original_app);
    ret = ISFS_Rename(temp_dest,original_app);
    if(ret < 0 )
    {
        gprintf("WritePriiloader : rename returned %d\n",ret);
        nand_copy(copy_app,original_app,SysPerm);
        ISFS_Delete(copy_app);
        abort("\nFailed to Write Priiloader : error Ren %d",ret);
    }
    printf("Done!!\n");
    gprintf("Wrote Priiloader App.Checking Installation\n");
    printf("\nChecking Priiloader Installation...\n");
    memset(status,0,sizeof(fstats));
    fd = ISFS_Open(original_app,ISFS_OPEN_READ);
    if (fd < 0)
    {
        nand_copy(copy_app,original_app,SysPerm);
        ISFS_Delete(copy_app);
        abort("\nFailed to open file for Priiloader checking");
    }
    if (ISFS_GetFileStats(fd,status) < 0)
    {
        ISFS_Close(fd);
        nand_copy(copy_app,original_app,SysPerm);
        abort("Failed to get stats of %s. System Menu Recovered",original_app);
    }
    else
    {
        if ( status->file_length != priiloader_app_size )
        {
            ISFS_Close(fd);
            nand_copy(copy_app,original_app,SysPerm);
            ISFS_Delete(copy_app);
            abort("Written Priiloader app isn't the correct size.System Menu Recovered");
        }
        else
        {
            gprintf("Size Check Success\n");
            printf("Size Check Success!\n");
        }
    }
    AppData = (u8 *)memalign(32,ALIGN32(status->file_length));
    if (AppData != NULL)
        ret = ISFS_Read(fd,AppData,status->file_length);
    else
    {
        ISFS_Close(fd);
        nand_copy(copy_app,original_app,SysPerm);
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure! MemAlign Failure of AppData\n");
    }
    ISFS_Close(fd);
    if (ret < 0)
    {
        if (AppData)
        {
            free(AppData);
            AppData = NULL;
        }
        nand_copy(copy_app,original_app,SysPerm);
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure! read of priiloader app returned %u\n",ret);
    }
    if(CompareSha1Hash((u8*)priiloader_app,priiloader_app_size,AppData,status->file_length))
        printf("Checksum comparison Success!\n");
    else
    {
        if (AppData)
        {
            free(AppData);
            AppData = NULL;
        }
        nand_copy(copy_app,original_app,SysPerm);
        ISFS_Delete(copy_app);
        abort("Checksum comparison Failure!\n");
    }
    if (AppData)
    {
        free(AppData);
        AppData = NULL;
    }
    gprintf("Priiloader Update Complete\n");
    printf("Done!\n\n");
    return 1;
}
s8 RemovePriiloader ( void )
{
    s32 fd = 0;
    Nand_Permissions SysPerm;
    memset(&SysPerm,0,sizeof(Nand_Permissions));
    SysPerm.otherperm = 3;
    SysPerm.groupperm = 3;
    SysPerm.ownerperm = 3;
    printf("Restoring System menu app...");
    s32 ret = nand_copy(copy_app,original_app,SysPerm);
    if (ret < 0)
    {
        if(ret == -80)
        {
            //checksum issues
            printf("\x1b[%u;%dm", 33, 1);
            printf("\nWARNING!!\n  Installer could not calculate the Checksum when coping the System menu app\n");
            printf("reverting changes...\n");
            ISFS_Close(fd);
            ISFS_CreateFile(original_app,0,3,3,3);
            fd = ISFS_Open(original_app,ISFS_OPEN_RW);
            ISFS_Write(fd,priiloader_app,priiloader_app_size);
            ISFS_Close(fd);
            abort("System Menu Copying Failure");
        }
        else
        {
            ISFS_CreateFile(original_app,0,3,3,3);
            fd = ISFS_Open(original_app,ISFS_OPEN_RW);
            ISFS_Write(fd,priiloader_app,priiloader_app_size);
            ISFS_Close(fd);
            abort("\nUnable to restore the system menu! (ret = %d)",ret);
        }
    }
    ret = ISFS_Delete(copy_app);
    printf("Done!\n");
    return 1;
}

void installPriiloader(bool vga) {
    s32 ret;
    //Kill everything we have initialized before reloading IOS
    shutdown();

    IOS_ReloadIOS(236);

    u64 TitleID = 0;
    u32 keyId = 0;
    ret = ES_Identify( (signed_blob*)haxx_certs, haxx_certs_size, (signed_blob*)su_tmd, su_tmd_size, (signed_blob*)su_tik, su_tik_size, &keyId);
    if (ret < 0) {
        printf("IOS isn't ES_Identify patched\n");
        exit(1);
    }

    ret = ES_GetTitleID(&TitleID);
    gprintf("identified as = 0x%08X%08X\n",TITLE_UPPER(TitleID),TITLE_LOWER(TitleID));

    if (ISFS_Initialize() < 0) {
        printf("Failed to init ISFS\n");
        exit(1);
    }

    if (!HaveNandPermissions()) {
        printf("IOS doesn't have NAND write permissions\n");
        exit(1);
    }

    //read TMD so we can get the main booting dol
    s32 fd = 0;
    u32 id = 0;
    ret = 0;
    memset(TMD_Path,0,64);
    memset(TMD_Path2,0,64);
    sprintf(TMD_Path, "/title/%08x/%08x/content/title.tmd",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
    sprintf(TMD_Path2, "/title/%08x/%08x/content/title_or.tmd",TITLE_UPPER(title_id),TITLE_LOWER(title_id));
    fd = ES_GetStoredTMDSize(title_id,&tmd_size);
    if (fd < 0)
    {
        printf("\x1b[2J");
        fflush(stdout);
        abort("Unable to get stored tmd size");
    }
    mTMD = (signed_blob *)tmd_buf;
    fd = ES_GetStoredTMD(title_id,mTMD,tmd_size);
    if (fd < 0)
    {
        printf("\x1b[2J");
        fflush(stdout);
        abort("Unable to get stored tmd");
    }
    rTMD = (tmd*)SIGNATURE_PAYLOAD(mTMD);
    for(u8 i=0; i < rTMD->num_contents; ++i)
    {
        if (rTMD->contents[i].index == rTMD->boot_index)
        {
            id = rTMD->contents[i].cid;
            break;
        }
    }
    if (id == 0)
    {
        printf("\x1b[2J");
        fflush(stdout);
        abort("Unable to retrieve title booting app");
    }

    memset(original_app,0,64);
    memset(copy_app,0,64);
    sprintf(original_app, "/title/%08x/%08x/content/%08x.app",TITLE_UPPER(title_id),TITLE_LOWER(title_id),id);
    sprintf(copy_app, "/title/%08x/%08x/content/%08x.app",TITLE_UPPER(title_id),TITLE_LOWER(title_id),id);
    copy_app[33] = '1';
    gprintf("%s &\n%s \n",original_app,copy_app);

    bool _Prii_Found = CheckForPriiloader();
    CopyTicket();
    WritePriiloader(_Prii_Found);
    ret = PatchTMD(0);

    if (_Prii_Found) {
        printf("Deleting extra priiloader files...\n");
        Delete_Priiloader_Files(1);
    }
    else if (!_Prii_Found) {
        printf("Attempting to delete leftover files...\n");
        Delete_Priiloader_Files(0);
    }


    //Copy RVL bootloader to main.bin
    //Check if there is already a main.dol installed
    fd = ISFS_Open("/title/00000001/00000002/data/main.bin", 1|2 );

    if( fd >= 0 )   //delete old file
    {
        ISFS_Close( fd );
        ISFS_Delete("/title/00000001/00000002/data/main.bin");
    }

    //file not found create a new one
    ISFS_CreateFile("/title/00000001/00000002/data/main.bin", 0, 3, 3, 3);
    fd = ISFS_Open("/title/00000001/00000002/data/main.bin", 1|2 );
    if (fd >= 0) {
        u8* dolbuf = (u8*)memalign(32, bootloader_dol_size);
        memcpy(dolbuf, bootloader_dol, bootloader_dol_size);
        ISFS_Write(fd, dolbuf, bootloader_dol_size);
        ISFS_Close(fd);
    }

    //Create main.nfo to patch AHBPROT when booting RVL
    fd = ISFS_Open("/title/00000001/00000002/data/main.nfo", 1|2 );

    if( fd >= 0 )   //delete old file
    {
        ISFS_Close( fd );
        ISFS_Delete("/title/00000001/00000002/data/main.nfo");
    }

    //file not found create a new one
    ISFS_CreateFile("/title/00000001/00000002/data/main.nfo", 0, 3, 3, 3);
    fd = ISFS_Open("/title/00000001/00000002/data/main.nfo", 1|2 );
    if (fd >= 0) {
        u8* nfobuf = (u8*)memalign(32, 7);
        memset(nfobuf, 0, 7);
        nfobuf[0] = 1;
        nfobuf[5] = 1;
        ISFS_Write(fd, nfobuf, 6);
        ISFS_Close(fd);
    }

    //Create loader.ini
    fd = ISFS_Open("/title/00000001/00000002/data/loader.ini", 1|2 );

    if( fd >= 0 )   //delete old file
    {
        ISFS_Close( fd );
        ISFS_Delete("/title/00000001/00000002/data/loader.ini");
    }

    //file not found create a new one
    ISFS_CreateFile("/title/00000001/00000002/data/loader.ini", 0, 3, 3, 3);
    fd = ISFS_Open("/title/00000001/00000002/data/loader.ini", 1|2 );
    if (fd >= 0) {
        Settings* settings = (Settings*)memalign( 32, ALIGN32(sizeof(Settings)) );
        memset( settings, 0, sizeof( Settings ) );
        settings->BetaVersion = 0x00000000;
        settings->version = 0x00000052;
        settings->UseSystemMenuIOS = 1;
        settings->autoboot = AUTOBOOT_FILE;
        settings->ReturnTo = RETURNTO_AUTOBOOT;
        settings->BlackBackground = 1;
        ISFS_Write(fd, settings, sizeof(Settings));
        ISFS_Close(fd);
    }

    //Copy RVL bootloader to main.bin
    //Check if there is already a main.dol installed
    fd = ISFS_Open("/title/00000001/00000002/data/vga.bin", 1|2 );

    if( fd >= 0 )   //delete old file
    {
        ISFS_Close( fd );
        ISFS_Delete("/title/00000001/00000002/data/vga.bin");
    }

    //file not found create a new one
    if (vga) {
        printf("Injecting VGA\n");
        ISFS_CreateFile("/title/00000001/00000002/data/vga.bin", 0, 3, 3, 3);
    }


    //Inject 480p in SYSCONF if needed
    fd = ISFS_Open("/shared2/sys/SYSCONF", 1|2);

    printf("Injecting 480p setting\n");
    if(fd >= 0) {
        u16 nEntries = 0;
        u16 pgsOffset = 0;
        bool pgsFound = true;
        u16 e60Offset = 0;
        bool e60Found = true;
        u8 pgsBuffer[9] = {0x66, 'I', 'P', 'L', '.', 'P', 'G', 'S', 0};
        u8 e60Buffer[9] = {0x66, 'I', 'P', 'L', '.', 'E', '6', '0', 0};
        u8* confBuffer = (u8*)memalign(0x20, 0x4000);
        ISFS_Read(fd, confBuffer, 0x4000);
        ISFS_Close(fd);

        //Check if IPL.PGS is present
        pgsOffset = *((u16*)(confBuffer + 0x3FDE));
        gprintf("pgsOffset: %u\n", pgsOffset);
        //IPL.PGS must be created
        if (!pgsOffset) {
            pgsFound = false;
            u16 firstEntryOffset = *((u16*)(confBuffer + 6));
            u16 firstFreeEntryOffset = 0;
            nEntries = *((u16*)(confBuffer + 4));
            gprintf("nEntries: %u\n", nEntries);

            //Increase every offset by 2 to make space for the new entry offset
            for (int i = 0; i < nEntries; i++) {
                *((u16*)(confBuffer + 6 + 2 * i)) += 2;
            }

            firstFreeEntryOffset = *((u16*)(confBuffer + 6 + 2 * nEntries));
            gprintf("firstEntryOffset: %04X\n", firstEntryOffset);
            gprintf("firstFreeEntryOffset: %04X\n", firstFreeEntryOffset);

            //Read current config and rewrite it to a shifted offset
            for (u16 off = firstFreeEntryOffset - 1; off >= firstEntryOffset; off--) {
                confBuffer[off + 2] = confBuffer[off];
            }

            //Write offset for new entry + past conf offset
            *((u16*)(confBuffer + 6 + 2 * nEntries)) += 2;
            *((u16*)(confBuffer + 6 + 2 * (nEntries + 1))) = *((u16*)(confBuffer + 6 + 2 * nEntries)) + sizeof(pgsBuffer);

            //Write index offset to LUT
            *((u16*)(confBuffer + 0x3FDE)) = 6 + 2 * nEntries;
            pgsOffset = 6 + 2 * nEntries;
            gprintf("pgsOffset: %04X\n", pgsOffset);

            //Increase number of entries
            nEntries++;
            *((u16*)(confBuffer + 4)) = nEntries;
        }

        //Check if IPL.PGS is present
        e60Offset = *((u16*)(confBuffer + 0x3FEE));
        gprintf("e60Offset: %u\n", e60Offset);
        //IPL.PGS must be created
        if (!e60Offset && CONF_GetVideo() == CONF_VIDEO_PAL) {
            e60Found = false;
            u16 firstEntryOffset = *((u16*)(confBuffer + 6));
            u16 firstFreeEntryOffset = 0;
            nEntries = *((u16*)(confBuffer + 4));
            gprintf("nEntries: %u\n", nEntries);

            //Increase every offset by 2 to make space for the new entry offset
            for (int i = 0; i < nEntries; i++) {
                *((u16*)(confBuffer + 6 + 2 * i)) += 2;
            }

            firstFreeEntryOffset = *((u16*)(confBuffer + 6 + 2 * nEntries));
            gprintf("firstEntryOffset: %04X\n", firstEntryOffset);
            gprintf("firstFreeEntryOffset: %04X\n", firstFreeEntryOffset);

            //Read current config and rewrite it to a shifted offset
            for (u16 off = firstFreeEntryOffset - 1; off >= firstEntryOffset; off--) {
                confBuffer[off + 2] = confBuffer[off];
            }

            //Write offset for new entry + past conf offset
            *((u16*)(confBuffer + 6 + 2 * nEntries)) += 2;
            *((u16*)(confBuffer + 6 + 2 * (nEntries + 1))) = *((u16*)(confBuffer + 6 + 2 * nEntries)) + sizeof(e60Buffer);

            //Write index offset to LUT
            *((u16*)(confBuffer + 0x3FEE)) = 6 + 2 * nEntries;
            e60Offset = 6 + 2 * nEntries;
            gprintf("e60Offset: %04X\n", e60Offset);

            //Increase number of entries
            nEntries++;
            *((u16*)(confBuffer + 4)) = nEntries;
        }

        //Update values
        pgsOffset = *((u16*)(confBuffer + pgsOffset));
        gprintf("pgsOffset: %04X\n", pgsOffset);
        if (vga)
            pgsBuffer[8] = 1;
        if (vga || !pgsFound)
            memcpy(&confBuffer[pgsOffset], pgsBuffer, sizeof(pgsBuffer));

        if (CONF_GetVideo() == CONF_VIDEO_PAL) {
            e60Offset = *((u16*)(confBuffer + e60Offset));
            gprintf("e60Offset: %04X\n", e60Offset);
            if (vga)
                e60Buffer[8] = 1;
            if (vga || !e60Found)
                memcpy(&confBuffer[e60Offset], e60Buffer, sizeof(e60Buffer));
        }

        //Write new config
        ISFS_Delete("/shared2/sys/SYSCONF");
        ISFS_CreateFile("/shared2/sys/SYSCONF", 0, 3, 3, 3);
        fd = ISFS_Open("/shared2/sys/SYSCONF", 1|2);
        if (fd >= 0) {
            ISFS_Write(fd, confBuffer, 0x4000);
            ISFS_Close(fd);
        }
    }

    abort("Install complete\n");
}
