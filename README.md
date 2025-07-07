# Directory_Sync_Daemon
a daemon that synchronises two directories



## 👥 Authors Personal Information

    *Author:* Barış GEÇER
    *Email:* 94065@student.pb.edu.pl
    *Affiliation:* Bialystok University of Technology
---

# Sync Daemon

A POSIX-compliant file synchronization daemon written in C. It synchronizes two directories by copying new or modified files from the source to the destination and removing files from the destination that no longer exist in the source.

---

## 🧠 How It Works

- Runs as a background **daemon** process.
- Synchronizes files every 5 minutes (can be changed via `SLEEP_TIME`).
- Handles both regular files and directories (with recursive mode).
- Logs all actions to **syslog** under the identifier `"Sync-Daemon"`.

---

## 📦 Features

- One-way sync (source → destination).
- Recursive mode via `-R` flag.
- Preserves modification times using `utime()`.
- Automatically removes outdated or deleted files/directories from the destination.
- Daemonized for continuous background execution.

---

## Function Documentation

int main(int argc, char const *argv[])
The entry point. Validates input, initializes the daemon, and starts the sync loop.

void mainLoop(const char *source, const char *destination, int recursive)
Infinite loop that calls syncDirectories() every SLEEP_TIME seconds.

int isValidDirectory(const char *path)
Checks if a directory exists and is accessible.

void logMessage(const char *message)
Writes a message to the system logger (syslog) under the "Sync-Daemon" tag.

void daemonStructure()
Transforms the process into a daemon using fork(), setsid(), chdir(), and closes standard file descriptors.

void copyFile(const char *sourcePath, const char *destPath)
Copies a file from source to destination. Also preserves modification times.

void removeFile(const char *path)
Deletes a file and logs the action.

void syncDirectories(const char *sourceDir, const char *destDir, int recursive)
The core logic:

Iterates through files in the source directory.

Copies missing or newer files to the destination.

Removes files or directories from the destination that don't exist in the source.

void removeDirectory(const char *path)
Removes an empty directory. Used when syncing and the source directory no longer contains it.

void createDirectory(const char *path)
Creates a directory with 0777 permissions if it doesn't exist in the destination.
---



## 🛠 Build Instructions

```bash
mkdir build
cd build
cmake ..
make

