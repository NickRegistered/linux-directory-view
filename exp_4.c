#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <string.h>

void printfile(const struct stat st_buf){

    struct passwd *pw;
    struct group *gr;
    struct tm *tm;

    int i;

    //文件类型
    switch(st_buf.st_mode & S_IFMT){
        case S_IFREG: putchar('-');break;
        case S_IFDIR: putchar('d');break;
        case S_IFLNK: putchar('l');break;
        case S_IFCHR: putchar('c');break;
        case S_IFIFO: putchar('p');break;
	case S_IFSOCK:putchar('s');break;
    }

    //权限
    for(i = 8;i >= 0;--i){
        if(st_buf.st_mode & (1 << i)){
            switch(i%3){
                case 2: putchar('r');break;
                case 1: putchar('w');break;
                case 0: putchar('x');break;
            }
        }
        else putchar('-');
    }

    //用户名、组名
    pw = getpwuid(st_buf.st_uid);
    gr = getgrgid(st_buf.st_gid);
    tm = localtime(&st_buf.st_ctim.tv_sec);

    printf("%2ld %s %s %8ld ",st_buf.st_nlink,
                            pw->pw_name,gr->gr_name,
                            st_buf.st_size);
    printf("%04d-%02d-%02d %02d:%02d",tm->tm_year + 1900,
                                    tm->tm_mon + 1,tm->tm_mday,
                                    tm->tm_hour,tm->tm_min);
    return;
}

void printdir(const char *dir,int depth){
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    //打开目录失败
    if((dp = opendir(dir)) == NULL){
        printf("open dir error!\n");
        exit(-1);
    }

    //切换打开目录为当前目录
    chdir(dir);

    //依次处理读到的目录项
    while(entry = readdir(dp)){
	//调用lstat获得目录stat结构
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)){//如果是目录
            if(entry->d_name[0] == '.')continue;//跳过.、..目录
	    //打印目录项的深度、目录名等信息
            printf("%*sdepth:%-2d ",depth/2+1," ",depth);
            printfile(statbuf);
            printf(" %s\n",entry->d_name);
	    //递归调用printdir、打印子目录信息
            printdir(entry->d_name,depth + 4);
        }
        else{//不算目录，打印文件深度等信息
            printf("%*sdepth:%-2d ",depth/2+1," ",depth);
            printfile(statbuf);
            printf(" %s\n",entry->d_name);
        }
    }

    chdir("..");
    closedir(dp);
    return;
}


int main(int argc,char **argv)
{
    if(argc == 2) printdir(argv[1],0);
    else printf("命令格式错误\n");
    return 0;
}
