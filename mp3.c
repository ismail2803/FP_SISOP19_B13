#include <ao/ao.h>
#include <mpg123.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#define BITS 8
static const char *dirpath = "/home/ismail/sisop/FP_SISOP19_B13/FP";
mpg123_handle *mh;
unsigned char *buffer;
ao_device *dev;
int key=0;
char list[1000][1000];
char playlist[1000][10000];
int i=0;
int no=0;

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void *pilih(void *args){
    while(1){
        while(!kbhit());
        key = getchar() - '0';
    }
}

void *putar(void *args){
    while(1){
        size_t buffer_size;
        size_t done;
        int err;

        int driver;

        ao_sample_format format;
        int channels, encoding;
        long rate;

        /* initializations */
        ao_initialize();
        driver = ao_default_driver_id();
        mpg123_init();
        mh = mpg123_new(NULL, &err);
        buffer_size = mpg123_outblock(mh);
        buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

        /* open the file and get the decoding format */
        mpg123_open(mh, list[no]);
        mpg123_getformat(mh, &rate, &channels, &encoding);

        /* set the output format and open the output device */
        format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
        format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = 0;
        dev = ao_open_live(driver, &format, NULL);

        /* decode and play */
        int flag=1;
        while(1){
            printf(" \b\b\b\b\b");
            if(key == 1){
                if(mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
                    ao_play(dev, buffer, done);
                }else{
                    no++;
                    no = no % i;
                    break;
                }
            }
            if(key == 3){
                key = 1;
                flag == 0;
                no++;
                no = no % i;
                break;
            }
            if(key == 4){
                key = 1;
                flag == 0;
                no--;
                if(no < 0)
                    no = i + no;
                break;
            }
        }
    }

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}

int main(int argc, char *argv[])
{
    // listdir(dirpath);
    DIR *d;

    struct dirent *dir;
    d = opendir(dirpath);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char nama[1000];
            memset(nama, '\0', sizeof(nama));

            sprintf(nama, "%s/%s", dirpath, dir->d_name);
            
            if(strcmp(dir->d_name, ".")!=0 && strcmp(dir->d_name, "..")!=0){
                strcpy(list[i], nama);
                strcpy(playlist[i], dir->d_name);
                i++;
                // printf("[-] %s\n", dir->d_name);
            }
        }
        closedir(d);
    }

    pthread_t thread1, thread2, thread3, thread4, thread5, thread6, thread7, thread8;
    int iret1, iret2, iret3, iret4, iret5, iret6, iret7, iret8;

    pthread_create( &thread1, NULL, &pilih, NULL);
    pthread_create( &thread1, NULL, &putar, NULL);

    while(1){
        for(int j=0; j<i; j++){
            if(j==no)
                printf("[*] %s\n", playlist[j]);
            else
                printf("[-] %s\n", playlist[j]);
        }
        sleep(1);
        system("clear");
    }
    return 0;
}