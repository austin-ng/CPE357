#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define TAR_BLK_SIZE 512
#define AFTER_PREFIX 500
#define MAX_PATH_LEN 256

/*List of macros defining the header field lengths*/
#define NAME_LEN 100
#define MODE_LEN 8
#define UID_LEN 8
#define GID_LEN 8
#define SIZE_LEN 12
#define MTIME_LEN 12
#define CHKSUM_LEN 8
#define TYPEFLAG_LEN 1
#define LINKNAME_LEN 100
#define MAGIC_LEN 6
#define VERSION_LEN 2
#define UNAME_LEN 32
#define GNAME_LEN 32
#define DEVMAJOR_LEN 8
#define DEVMINOR_LEN 8
#define PREFIX_LEN 155

int v; /*Boolean of whether verbose option was selected by user*/
int strict; /*Boolean of whether strict option was selected by user*/
int tar_block_size; /*Number of blocks currently inside tar file*/

int writeHeader(int fd, char* filename, char* prefix, struct stat s);
void writeContents(int fd_tar, int fd_file);

int insert_special_int(char* where, size_t size, int32_t val) {
    int err = 0;

    if ((val < 0) || (size < sizeof(val))) {
	err++;
    }
    else {
        memset(where, 0, size);
        *(int32_t*) (where + size - sizeof(val)) = htonl(val);
        *where |= 0x80;
    }

    return err;
}

void makeOctalString(int32_t val, char* where) {
    int max_eight_oct = 07777777;
    memset(where, '\0', UID_LEN);
    if (val <= max_eight_oct) {
        sprintf(where, "%07o", val);
    }
    else {
	if (strict) {
	    fprintf(stderr, "%d doesn't fit octal string. Unable to archive",
			    val);
	}
	else if (insert_special_int(where, UID_LEN, val) != 0) {
	    fprintf(stderr, "Unable to write special 8 int");
	}
    }
}

char* getPrefix(const char* filename) {
    char* prefix_name;
    char* post_name_start;
    int p_name_len;

    char* temp_filename = malloc(strlen(filename) + 1);
    strcpy(temp_filename, filename);

    if (strlen(filename) <= NAME_LEN) {
	free(temp_filename);
	return NULL;
    }

    p_name_len = strlen(filename) - NAME_LEN;
    temp_filename += p_name_len;

    post_name_start = strchr(temp_filename, '/');

    temp_filename -= p_name_len;

    if (post_name_start) {
	p_name_len = strlen(filename) - strlen(post_name_start);
    }
    else {
	free(temp_filename);
	return NULL;
    }

    prefix_name = malloc(p_name_len + 1);
    memset(prefix_name, '\0', p_name_len + 1);
    memcpy(prefix_name, filename, p_name_len);

    free(temp_filename);    
    return prefix_name;
}  

void writeDirectory(int fd_tar, char* fname) {
    DIR* dir;
    struct dirent* dp;
    struct stat statbuf;
    int res;
    int new_file_fd;
    
    int file_dir_pos = 0;
    char* pathname;
    char pathname_cpy[MAX_PATH_LEN];

    if ((dir = opendir(fname)) == NULL) {
	perror(fname);
        return;
    }

    pathname = strdup(fname);
    while ((dp = readdir(dir)) != NULL) {
        if (file_dir_pos > 1) {
	    memset(pathname_cpy, '\0', MAX_PATH_LEN);
	    strncpy(pathname_cpy, pathname, MAX_PATH_LEN);
            if ((strlen(pathname) + strlen(dp->d_name)) > MAX_PATH_LEN) {
		fprintf(stderr, "%s: New path name too long. Can't archive", 
				 dp->d_name);
	    }
	    else {		
	        strcat(pathname_cpy, dp->d_name);
                if ((new_file_fd = open(pathname_cpy, O_RDONLY)) < 0) {
	            perror(pathname_cpy);
	            break;
	    	}
            	else { 
		    if (lstat(pathname_cpy, &statbuf) < 0) {
	            	perror(pathname_cpy);
	            	break;
	            }
	            res = writeHeader(fd_tar, pathname_cpy, 
			              getPrefix(pathname_cpy), statbuf);
                    if (res == 0) writeContents(fd_tar, new_file_fd);
                }

	        close(new_file_fd);
	    }

	}
	file_dir_pos++;
    }

    if (closedir(dir) < 0) {
	fprintf(stderr, "Unable to close directory: %s", fname);
    }

    free(pathname);
}

int writeHeader(int fd, char* filename, char* prefix, struct stat s) {
    /*Variables with _w represent what is actually written to the tar file*/
    char name_w[NAME_LEN]; char mode_w[MODE_LEN]; char uid_w[UID_LEN];
    char gid_w[GID_LEN]; char size_w[SIZE_LEN]; char mtime_w[MTIME_LEN];
    char chksum_w[CHKSUM_LEN]; char typeflag_w; char linkname_w[LINKNAME_LEN];
    char magic_w[MAGIC_LEN]; char version_w[VERSION_LEN]; 
    char uname_w[UNAME_LEN]; char gname_w[GNAME_LEN]; 
    char devmajor_w[DEVMAJOR_LEN]; char devminor_w[DEVMINOR_LEN];
    char prefix_w[PREFIX_LEN];

    int i;
    char* filename_copy;
    char* dir_name;
    char check_lname[LINKNAME_LEN + 2];
    struct passwd* user_pwd;
    struct group* group_grp;
    int numchars;

    int err = 0;
    int chksum_offset = 148;
    unsigned char header_buf[AFTER_PREFIX];
    int chksum_int = 0;
    char end_of_header[SIZE_LEN] = {'\0'};

    filename_copy = malloc(MAX_PATH_LEN);

    memset(name_w, '\0', NAME_LEN); /*Name to write*/
    dir_name = malloc(MAX_PATH_LEN);
    strcpy(filename_copy, filename);
    if (prefix) {
	filename_copy += (strlen(prefix) + 1);
        if (S_ISDIR(s.st_mode)) {
            strcpy(dir_name, filename);
	    strcat(dir_name, "/");
	    strcat(filename_copy, "/");
	}
        strncpy(name_w, filename_copy, NAME_LEN);
	filename_copy -= (strlen(prefix) + 1);
    }
    else {
        if (S_ISDIR(s.st_mode)) {
	    strcpy(dir_name, filename);
	    strcat(dir_name, "/");
	}
	strncpy(name_w, ((S_ISDIR(s.st_mode))? dir_name : filename), NAME_LEN);
    }

    free(filename_copy); /*RECENTLY ADDED*/

    memset(mode_w, '\0', MODE_LEN); /*Mode to write*/
    sprintf(mode_w, "%07o", s.st_mode & 0777);
    
    makeOctalString((int32_t) s.st_uid, uid_w); /*UID to write*/

    makeOctalString((int32_t) s.st_gid, gid_w); /*GID to write*/

    memset(size_w, '\0', SIZE_LEN); /*Size to write*/
    sprintf(size_w, "%011o", (S_ISREG(s.st_mode)? (int) s.st_size : 0));

    memset(mtime_w, '\0', MTIME_LEN); /*Mtime to write*/
    sprintf(mtime_w, "%011o", (int) s.st_mtime);

    sprintf(chksum_w, "        "); /*Initial condition for checksum*/

    typeflag_w = '\0'; /*Typeflag to write*/
    if S_ISREG(s.st_mode) typeflag_w = '0';
    else if S_ISDIR(s.st_mode) typeflag_w = '5';
    else if S_ISLNK(s.st_mode) typeflag_w = '2';

    memset(linkname_w, '\0', LINKNAME_LEN); /*Linkname to write*/
    memset(check_lname, '\0', LINKNAME_LEN + 2);
    if S_ISLNK(s.st_mode) {
        if (readlink(filename, check_lname, LINKNAME_LEN + 2) < 0) {
	    perror(filename);
	    err++;
        }
        else {
            if (strlen(check_lname) > LINKNAME_LEN) {
	        fprintf(stderr, "Linkname too long for header\n");
	        err++;
	    }
	    else {
		strcpy(linkname_w, check_lname);
	    }
	}
    }

    memset(magic_w, '\0', MAGIC_LEN);
    strcpy(magic_w, "ustar");

    memset(version_w, '0', VERSION_LEN);

    memset(uname_w, '\0', UNAME_LEN);
    user_pwd = getpwuid(s.st_uid);
    strncpy(uname_w, user_pwd->pw_name, UNAME_LEN - 1);

    memset(gname_w, '\0', GNAME_LEN);
    group_grp = getgrgid(s.st_gid);
    strncpy(gname_w, group_grp->gr_name, GNAME_LEN - 1);

    memset(devmajor_w, '\0', DEVMAJOR_LEN);
    memset(devminor_w, '\0', DEVMINOR_LEN);

    memset(prefix_w, '\0', PREFIX_LEN);
    if (prefix) {
	strncpy(prefix_w, prefix, PREFIX_LEN);
    }

    /*Writing all the header fields (excluding correct chksum*/
    if (err == 0) {
        lseek(fd, 0, SEEK_END);
        write(fd, &name_w, NAME_LEN); write(fd, &mode_w, MODE_LEN);
        write(fd, &uid_w, UID_LEN); write(fd, &gid_w, GID_LEN);
        write(fd, &size_w, SIZE_LEN); write(fd, &mtime_w, MTIME_LEN);
        write(fd, &chksum_w, CHKSUM_LEN); write(fd, &typeflag_w, TYPEFLAG_LEN);
	write(fd, &linkname_w, LINKNAME_LEN); write(fd, &magic_w, MAGIC_LEN);
	write(fd, &version_w, VERSION_LEN); write(fd, &uname_w, UNAME_LEN);
	write(fd, &gname_w, GNAME_LEN); write(fd, &devmajor_w, DEVMAJOR_LEN);
	write(fd, &devminor_w, DEVMINOR_LEN); write(fd, &prefix_w, PREFIX_LEN);
        write(fd, &end_of_header, SIZE_LEN);

        /*Writing the chksum field*/
        lseek(fd, (TAR_BLK_SIZE * tar_block_size), SEEK_SET);
        if ((numchars = read(fd, &header_buf, TAR_BLK_SIZE)) < 0) {
	    fprintf(stderr, "Unable to find chksum");
	    err++;
        }

        for (i = 0; i < numchars; i++) {
	    chksum_int += header_buf[i];
        }

        makeOctalString(chksum_int, chksum_w);
        lseek(fd, chksum_offset + (TAR_BLK_SIZE * tar_block_size), SEEK_SET);
        write(fd, &chksum_w, CHKSUM_LEN);
        tar_block_size++;
    }

    lseek(fd, 0, SEEK_END);
    if (!S_ISREG(s.st_mode)) {
	err++;
    }

    if (S_ISDIR(s.st_mode)) {
	if (v) printf("%s\n", dir_name);
	writeDirectory(fd, dir_name);
    }
    else {
	if (v && (err == 0)) {
	    if (prefix) {
		printf("%s/%s\n", prefix, name_w);
	    }
	    else {
	        printf("%s\n", name_w);
	    }
	}
    }

    if (prefix) free(prefix);
    free(dir_name);

    return err;   
}

void writeContents(int fd_tar, int fd_file) { 
    int numchars;
    int buf_max = TAR_BLK_SIZE;

    unsigned char* content_buf = malloc(TAR_BLK_SIZE + 1);
    memset(content_buf, '\0', TAR_BLK_SIZE + 1);

    while ((numchars = read(fd_file, content_buf, buf_max + 1)) > 0) {
	if (numchars == (buf_max + 1)) {
	    buf_max += TAR_BLK_SIZE;
	    content_buf = realloc(content_buf, buf_max + 1);
            memset(content_buf, '\0', buf_max + 1);
	    lseek(fd_file, 0, SEEK_SET);
	    tar_block_size++;
	}
	else {
	    write(fd_tar, content_buf, buf_max);
	    tar_block_size++;
	    break;
	}
    }

    if (numchars < 0) {
	fprintf(stderr, "Unable to write file contents to tar");
    }

    free(content_buf);
}

void createTar(char* filename, char* files_to_add[], int num_args, int isV, 
              int isS) {
    int tar_fd;
    int cur_fd;
    struct stat sb;
    int i;
    int res;
    
    /*Buffer representing End of Archive (EOA)*/
    char EOA[TAR_BLK_SIZE] = {'\0'};

    v = isV;
    strict = isS;

    if ((tar_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 
                       S_IRUSR | S_IWUSR)) < 0) {
	perror("createTar");
	exit(EXIT_FAILURE);
    }

    tar_block_size = 0;

    for (i = 3; i < num_args; i++) {
        if (strlen(files_to_add[i]) > MAX_PATH_LEN) {
	    fprintf(stderr, "%s: Path name too long. Unable to archive",
			    files_to_add[i]);
	}
        else {
	    if ((cur_fd = open(files_to_add[i], O_RDONLY)) < 0) {
	        perror(files_to_add[i]);
            }
            else {
                if (lstat(files_to_add[i], &sb) < 0) {
		    perror(files_to_add[i]);
	        }
	
	        res = writeHeader(tar_fd, files_to_add[i],
				  getPrefix(files_to_add[i]),  sb);
	        if (res == 0) writeContents(tar_fd, cur_fd);

                close(cur_fd);
	    }
	}
    }

    /*Writing the End of Archive: 2 512 byte block*/
    write(tar_fd, &EOA, TAR_BLK_SIZE);
    write(tar_fd, &EOA, TAR_BLK_SIZE);

    close(tar_fd);
}   
