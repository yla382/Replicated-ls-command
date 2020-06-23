#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


int iflag;
int lflag;
int pathCount;

int checkFlag(char* c) { //Check for the flags in the argv
	if(strcmp(c, "-i") == 0) {
		iflag = 1;
	} else if(strcmp(c, "-l") == 0) {
		lflag = 1;
	} else if(strcmp(c, "-il") == 0 || strcmp(c, "-li") == 0) {
		iflag = 1;
		lflag = 1;
	} else {
		return 0;
	}
	return 1;
}

void displayDate(time_t* time) {  //print dates
	struct tm *datetime = localtime(time);
	int month = datetime->tm_mon;

	//display Month
	if(month == 0) {
		printf("Jan ");
	} else if(month == 1) {
		printf("Feb ");
	} else if(month == 2) {
		printf("Mar ");
	} else if(month == 3) {
		printf("Apr ");
	} else if(month == 4) {
		printf("May ");
	} else if(month == 5) {
		printf("Jun ");
	} else if(month == 6) {
		printf("Jul ");
	} else if(month == 7) {
		printf("Aug ");
	} else if(month == 8) {
		printf("Sep ");
	} else if(month == 9) {
		printf("Oct ");
	} else if(month == 10) {
		printf("Nov ");
	} else {
		printf("Dec ");
	}

	//Display date
	if(datetime->tm_mday >= 10) {
		printf("%d ", datetime->tm_mday);
	} else {
		printf("0%d ", datetime->tm_mday);
	}

	//Display year
	printf("%d ", datetime->tm_year + 1900);

	//Display hour
	if(datetime->tm_hour >= 10) {
		printf("%d:", datetime->tm_hour);
	} else {
		printf("0%d:", datetime->tm_hour);
	}

	//Display minute
	if(datetime->tm_min >= 10) {
		printf("%d ", datetime->tm_min);
	} else {
		printf("0%d ", datetime->tm_min);
	}
}

void displayFlag(int flag1, int flag2, char *path, char *filename) {  //Print extra info based on the flags
	struct stat stats;
	struct passwd *pws;
	struct  group *grp;
	char *link;
	int linkSize;
	int endString;

	if(lstat(path, &stats) == -1) {
		printf("Error %s\n", path);
		return;
	}

	pws = getpwuid(stats.st_uid);
	grp = getgrgid(stats.st_gid);

	if(flag1 == 1) {
		printf("%-10lu ", stats.st_ino); //Display Inode number
	}

	char permissions[10];
	for(int i = 0; i < 10; i++) {
		permissions[i] = '-';
	}
	if(S_ISDIR(stats.st_mode)) {
		permissions[0] = 'd';
	}
	if(S_ISLNK(stats.st_mode)) {
		permissions[0] = 'l';
	}
	//User Permision
	if(stats.st_mode & S_IRUSR) {
		permissions[1] = 'r';
	}
	if(stats.st_mode & S_IWUSR) {
		permissions[2] = 'w';
	}
	if(stats.st_mode &S_IXUSR) {
		permissions[3] = 'x';
	}
	//Group permission
	if(stats.st_mode & S_IRGRP) {
		permissions[4] = 'r';
	}
	if(stats.st_mode & S_IWGRP) {
		permissions[5] = 'w';
	}
	if(stats.st_mode & S_IXGRP) {
		permissions[6] = 'x';
	}
	//Other permission
	if(stats.st_mode & S_IROTH) {
		permissions[7] = 'r';
	}
	if(stats.st_mode & S_IWOTH) {
		permissions[8] = 'w';
	}
	if(stats.st_mode & S_IXOTH) {
		permissions[9] = 'x';
	}


	if (stats.st_size == 0) {
		linkSize = PATH_MAX;
	} else {
		linkSize = stats.st_size + 1;
	}

	link = malloc(linkSize);
	endString = readlink(path, link, linkSize);
	link[endString] = '\0';
	if(flag2 == 1) {
		printf("%s ", permissions); //print permission of the file
		printf("%3lu ", stats.st_nlink); //print numbero f hard-linke pointing to the file
		printf("%s ", pws->pw_name); //print user name of teh file owner
		printf("%s ", grp->gr_name); //print group name the owner belongs to
		printf("%6ld ", stats.st_size); //print size of the file in bytes
		displayDate(&stats.st_mtime); //print the date and time when the file was last modified
		printf(" ");

		if(endString == -1) {
			//printf("%s", link);
			printf("%s\n", filename); //print file name
		} else {
			printf("%s->%s/\n", filename, link); //print file name and symbolic link
		}
	}

	if((flag1 == 0 || flag1 == 1) && flag2 == 0) {
		printf("%s\n", filename); //print file name
	}

	free(link);
}


void displayls(char* path) { //Iterate through the files in the path and call displayFlag
	DIR *directory;
	struct dirent *files;

	char* dir = (char*)(path);
	directory = opendir(dir);

	struct stat fileOrDir;
	stat(dir, &fileOrDir);
	if(S_ISREG(fileOrDir.st_mode)) { //check if the given path is a file or directory
		displayFlag(iflag, lflag, dir, dir);
	} else if(directory == NULL) {  //Check if directory can be successfully opened
		printf("unixls: cannot access '%s': No such file or directory\n", dir);
	} else {
		if(pathCount > 1) {
			printf("%s:\n", dir);
		}
		while((files = readdir(directory)) != NULL) {
			if(strcmp(files->d_name, ".") != 0 && strcmp(files->d_name, "..") != 0 && (files->d_name)[0] != '.') {
				char *pathname = malloc(strlen(dir) + strlen(files->d_name) + 2);
				sprintf(pathname, "%s/%s", dir, files->d_name); //Combine the given path and file name	
				displayFlag(iflag, lflag, pathname, files->d_name);
				free(pathname);
			}
		}
		closedir(directory);
		printf("\n");
	}
}


int main(int argc, char **argv) {
	iflag = 0;
	lflag = 0;
	pathCount = 0;

	for(int i = 1; i < argc; i++) { //Check for flag options in the argument
		if(!checkFlag(argv[i])) {
			pathCount++;
		} 
	}

	char *dot = ".";  //current directory
	if(pathCount == 0) { //If the path specified in argv use dot
		displayls(dot);
	} else {
		for(int i = 1; i < argc; i++) { //Search for Paths in the argv for displayls argument
			if(!checkFlag(argv[i])) {
				displayls(argv[i]);
			}
		}
	}

	return 0;
}
