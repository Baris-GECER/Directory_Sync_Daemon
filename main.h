#ifndef MAIN_H
#define MAIN_H


//5 minutes
#define SLEEP_TIME 60*5 

void mainLoop(const char *source, const char *destination , int recursive);

int isValidDirectory(const char *path);

void logMessage(const char *message);

void daemonStructure();

void copyFile(const char *sourcePath , const char *destPath);

void removeFile(const char *path);

void syncDirectories(const char *source_dir, const char *dest_dir , int recursive);

void removeDirectory(const char *path);

void createDirectory(const char *path);

#endif