#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <time.h>
#include <utime.h>

#include "main.h"

int main(int argc, char const *argv[])
{
	
	const char* sourcePath = argv[1];

	const char* destinationPath = argv[2];

	int recursive = 0;

    if (argc == 4 && strcmp(argv[3], "-R") == 0) {
        recursive = 1;  // Enable recursive sync
    }

	assert(isValidDirectory(sourcePath));
	assert(isValidDirectory(destinationPath));


	if(!strcmp(sourcePath,destinationPath)){
		perror("directories cant be same\n");
	}
	if (!isValidDirectory(sourcePath))
	{
		perror("Source path is not a valid path\n");
	}
	if (!isValidDirectory(destinationPath))
	{
		perror("Destinatin path is not a valid path\n");
	}

	daemonStructure();

    logMessage("Daemon Initialized");

    mainLoop( sourcePath, destinationPath ,recursive);

    

    closelog();
	exit(0);
}

void mainLoop(const char *source, const char *destination, int recursive){

	while(1){

		syncDirectories(source,destination,recursive);

		sleep(SLEEP_TIME);
		
	}

}

int isValidDirectory(const char *path){
	DIR *dir = opendir(path);
	if(dir){
		closedir(dir);
		return 1;
	}
	return 0;
}

void logMessage(const char *message){

	openlog("Sync-Daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE,"%s", message);
    closelog();

}

void daemonStructure(){

	pid_t pid , sid;

	pid = fork();

	if( pid < 0){
		logMessage("culdnt fork the proccess");
		perror("culdnt fork the proccess");
	}
	// exit the parent process
	if( pid > 0){
		exit(0);
	}

	umask(0);


	// get uniqe session ID
	sid = setsid();

	assert(sid >= 0);

	if(sid < 0){
		logMessage("couldn't get a valid session ID");
		perror("couldn't get a valid session ID");
	}

	if(chdir("/") < 0){
		logMessage("couldn't change working directory");
		perror("couldn't change working directory");
	}

	close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void copyFile(const char *sourcePath , const char *destPath){
    char buffer[1024];

    assert(sourcePath != NULL && destPath != NULL);

    size_t n;

    FILE *source = fopen( sourcePath, "rb");
    if (source == NULL) {
        perror("Error opening source file");
        

        char logMsg[sizeof(sourcePath) + sizeof(destPath) + 32];
    	//print to buffer
    	snprintf(logMsg, sizeof(logMsg), "Couldn't open the source file %s", sourcePath);
    	logMessage(logMsg);
        return;
    }

    FILE *dest = fopen(destPath, "wb");
    if (dest == NULL) {
        perror("Error opening destination file");
        
        char logMsg[sizeof(sourcePath) + sizeof(destPath) + 32];
    	snprintf(logMsg, sizeof(logMsg), "Couldn't open the destination file %s", destPath);
    	logMessage(logMsg);
        fclose(source);
        return;
    }

    while ((n = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, n, dest);
    }

    fclose(source);
    fclose(dest);

    struct stat st;

    if (stat(sourcePath, &st) == 0) {
        struct utimbuf times = {st.st_atime, st.st_mtime};
        utime(destPath, &times);
    }

    char logMsg[sizeof(sourcePath) + sizeof(destPath) + 32];

    //print to buffer
    snprintf(logMsg, sizeof(logMsg), "Copied '%s' to '%s'", sourcePath, destPath);
    logMessage(logMsg);

}

void removeFile(const char *path) {
    if (remove(path) == 0) {
    	char logMsg[sizeof(path) + 32];
        //print to buffer
        snprintf(logMsg, sizeof(logMsg), "Removed '%s' from destination.", path);
        logMessage(logMsg);
    } else {
        perror("Error removing file");

        char logMsg[sizeof(path) + 32];
        snprintf(logMsg, sizeof(logMsg) ,"Couldn't remove the file at %s" , path); 
        logMessage(logMsg);
    }
}



void syncDirectories(const char *sourceDir, const char *destDir, int recursive)
{
    DIR *source = opendir(sourceDir);
    if (source == NULL) {
        perror("Error opening source directory");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(source)) != NULL) {
        // Skip "." and ".."
        // "." is current dir
        // ".." is prev dir
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_path[sizeof(sourceDir) + sizeof(entry->d_name) + 32], dest_path[sizeof(destDir) + sizeof(entry->d_name) + 32];
        snprintf(source_path, sizeof(source_path), "%s/%s", sourceDir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", destDir, entry->d_name);

        struct stat source_stat;
        if (stat(source_path, &source_stat) != 0) {
            continue;  // Error or file doesn't exist
        }

        // Handle recursive
        if (S_ISDIR(source_stat.st_mode)) {
            if (recursive) {
                if (!isValidDirectory(dest_path)) {
                    createDirectory(dest_path);
                }
                syncDirectories(source_path, dest_path, recursive);// recursion into sub the dir
            }
            continue;
        }

        // check if regular
        if (!S_ISREG(source_stat.st_mode)) {
            continue;
        }

        struct stat dest_stat;
        int dest_exists = (stat(dest_path, &dest_stat) == 0);

        // if dest doesnt have the up to date file copt the file 
        if (!dest_exists || difftime(source_stat.st_mtime, dest_stat.st_mtime) > 0) {
            copyFile(source_path, dest_path);
        }
    }

    // chek if any file to remove, if there is than remove
    DIR *dest = opendir(destDir);
    if (dest == NULL) {
        perror("Error opening destination directory");
        closedir(source);
        return;
    }

    while ((entry = readdir(dest)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char source_path[sizeof(sourceDir) + sizeof(entry->d_name) + 32], dest_path[sizeof(destDir) + sizeof(entry->d_name) + 32];
        snprintf(source_path, sizeof(source_path), "%s/%s", sourceDir, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", destDir, entry->d_name);

        struct stat source_stat;
        int source_exists = (stat(source_path, &source_stat) == 0);

        // if source dont have it than remove
        if (!source_exists) {
            struct stat dest_stat;
            if (stat(dest_path, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
                removeDirectory(dest_path);  // remove directory
            } else {
                removeFile(dest_path);  // remove regular file
            }
        }
    }

    closedir(source);
    closedir(dest);
}

void removeDirectory(const char *path)
{
    if (rmdir(path) == 0) {
        char logMsg[sizeof(path) + 64];
        snprintf(logMsg, sizeof(logMsg), "Removed directory '%s' from destination.", path);
        logMessage(logMsg);
    } else {
        perror("Error removing directory");
        char logMsg[sizeof(path) + 64];
        snprintf(logMsg, sizeof(logMsg), "Couldn't remove the directory at %s", path);
        logMessage(logMsg);
    }
}

void createDirectory(const char *path)
{
	// 0777 sets that each user can read write and execute
    if (mkdir(path, 0777) == 0) {
        char logMsg[sizeof(path) + 64];
        snprintf(logMsg, sizeof(logMsg), "Created directory '%s' in destination.", path);
        logMessage(logMsg);
    } else {
        perror("Error creating directory");
        char logMsg[sizeof(path) + 64];
        snprintf(logMsg, sizeof(logMsg), "Couldn't create the directory at %s", path);
        logMessage(logMsg);
    }
}