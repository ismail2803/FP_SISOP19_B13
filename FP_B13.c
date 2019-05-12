#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/ismail";
char hapus[1000][1000];
int i=0;

void listdir(const char *name)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == 4) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

            listdir(path);
        } else {
            char file[1000];
            char *ext, c = '.';
            ext = strrchr(entry->d_name, c);
		    memset(file, '\0', sizeof(file));
            sprintf(file, "%s/%s", dirpath, entry->d_name);
            
            if(ext != NULL){
                if(strcmp(ext, ".mp3") == 0 && strcmp(name, "/home/ismail") != 0){
					char fileo[1000];
					memset(fileo, '\0', sizeof(fileo));
					sprintf(fileo, "%s/%s", name, entry->d_name);

					strcpy(hapus[i], file);
					i++;

					pid_t child_id;
    				child_id = fork();
					if(child_id == 0){
						char *argv[4] ={"cp", fileo, file, NULL};
						execv("/bin/cp", argv);
					}
                }    
            }            
        }
    }
    closedir(dir);
}

void xmp_destroy(void *private_data){
	for(int j=0; j<i; j++){
		remove(hapus[j]);
	}
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		char file[1000];
		memset(file, '\0', sizeof(file));

		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		if(de->d_type != 4 || strcmp(de->d_name, ".")==0 || strcmp(de->d_name, "..")==0)
        {
			if(de->d_type != 4){	
				char *ext, c = '.';
				ext = strrchr(de->d_name, c);
				
				if(ext != NULL){
					if(strcmp(ext, ".mp3") == 0){
						res = (filler(buf, de->d_name, &st, 0));
							if(res!=0) break;    
					}    
				}
			}else{
				res = (filler(buf, de->d_name, &st, 0));
					if(res!=0) break;
			}           
        }
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
    int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.destroy	= xmp_destroy,
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	listdir(dirpath);
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}