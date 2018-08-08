/*
1、 判断目标文件是否存在，如果存在则清空目标文件，如果不存在则创建目标文件 
2、根据目标文件的逻辑块大小，创建拷贝缓冲区 
3、判断源文件是否有空洞：文件大小／文件块大小 > 块数 ？ 
4、读取源文件存放到缓冲区，每次读取一块 
5、在第3步中判断，如果存在文件空洞，则对缓冲区数据进行判断，如果缓冲区中的数据均为0，则认为该数据快为空洞，否则认为是正常文件数据 
6、如果数据块为空洞，则调用lseek，在目标文件中创建一个空洞；否则拷贝缓冲区数据到目标文件 
7、判断本次读取是否读到源文件的文件尾，如果是，则判断本次读取的是否是空洞，如果是空洞则在文件的最后写入”” 
8、重复1 ~ 7 
9、关闭目标文件、源文件
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#define BUF_SIZE 4096

int my_cp(const char *file1, const char *file2)
{
    int fd1, fd2;
    char buffer[BUF_SIZE];
    int res, current_position = 0, byte_count =0, have_holes = 0;
    struct stat st;

    fd1 = open(file1, O_RDWR);
    if(-1 == fd1){
        perror("open file1 faild");
        return -1;
    }

    if(fstat ( fd1, &st) !=0)//information of file
        perror("fstat");
    else{
        if (S_ISREG(st.st_mode) && st.st_size > 512 * st.st_blocks) {
            //S_ISREG is to judge the normal file
            //blocks can let us know if there are holes
            have_holes = 1;
            printf("%s is a sparse-block file!\n", file1);
        } else{ 
            have_holes = 0;
            printf("%s is not a sparse-block file!\n", file1);
        }
    }

    fd2 = open(file2, O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 0666);
    if ( -1 == fd2) {
        perror ("open file2 faild");
        return -1;
    }

    memset(buffer, '\0', BUF_SIZE);
    res = read(fd1, buffer, BUF_SIZE);//返回读到的字节数
    if ( -1 == res) {
        perror ("file1 read error");
        return -1;
    }

    if(have_holes){
        byte_count =0;
        for (current_position = 0; current_position < res; current_position ++) {

            if (0 != buffer[current_position] )
            {
                    buffer[byte_count] = buffer[current_position];
                    //jump from the zero
                    byte_count ++;
            }
        }
    }else
        byte_count = res;

    res = write(fd2, buffer, byte_count);
    if ( -1 == res){
        perror( " file2 write error");
        return -1;
    }

    close(fd1);
    close(fd2);
}


int main(int argc, char * argv[])
{
    if (3 != argc){
        printf("usage error : ./a.out file1 file2\n");
        return -1;
    }
    int res = my_cp(argv[1], argv[2]);
    if ( -1 == res) {
        perror (" my_cp error");
        return -1;
    }

    exit(0);
}
