/*
**    UNIX 'du' implementation with additional flags and other doodads 
**    CSC 357 (Systems Programming) Final project for Prof. Andrew Wang @ Cal Poly
**    
**    Written by Elliot Kirk
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <unistd.h>

#define MAX_LEN 80
#define BLOCKSIZE 1024
#define ONE_K 1000

int checkDir(char *path);
void displayAll(DIR *directory, char *path, long *total, int silent, int depth, int human);
void displayFolders(DIR *directory, char *path, long *total, int depth, int human);
void findSub(DIR *directory, char *path, char *sub);
void goUp(int up, char *rtn);
int roundUp(int num);
void printHuman(struct stat *stats, long tot, char *curPath);

/*
   Prints all files in a directory and sums up total size, recursively calls sub-directories
   Printing can be turned off by passing in a non-zero value for silent
*/
void displayAll(DIR *directory, char *path, long *total, int silent, int depth, int human) {
   char *temp = malloc(sizeof(char) * MAX_LEN);
   char *curPath = malloc(sizeof(char) * MAX_LEN);
   struct dirent *tempdir = malloc(sizeof(struct dirent));
   struct stat *stats = malloc(sizeof(struct stat));

   if (directory) {
      while ((tempdir = readdir(directory)) ) {
         if (path == NULL)
            sprintf(curPath, "./%s", tempdir->d_name);
         else 
            sprintf(curPath, "%s/%s", path, tempdir->d_name);

         if (checkDir(tempdir->d_name) && strcmp(tempdir->d_name, "..") != 0 && 
          strcmp(tempdir->d_name, ".") != 0 && depth > 0 ) {
            sprintf(temp, "./%s", tempdir->d_name);
            chdir(tempdir->d_name);
            displayAll(opendir("."), curPath, total, silent, depth - 1, human);
            chdir("..");
         }
         if (strcmp(tempdir->d_name, "..") != 0 && strcmp(tempdir->d_name, ".") != 0 ) {
            lstat(tempdir->d_name, stats);
            if (!human)
               *total += roundUp((stats->st_size + (BLOCKSIZE - 1)) / BLOCKSIZE);
            if (human)
               *total += stats->st_size;
            if (!silent)
               printf("%d\t%s\n", roundUp((int)(stats->st_size + BLOCKSIZE - 1)/BLOCKSIZE), curPath);
            if (human)
               printHuman(stats, -1, curPath);
         }
      }
   }
}

/*
   Similar to displayAll(), but only prints out folders and the size of those folders
   Also recursively enters sub-directories
*/
void displayFolders(DIR *directory, char *path, long *total, int depth, int human) {
   char *temp = malloc(sizeof(char) * MAX_LEN);
   char *curPath = malloc(sizeof(char) * MAX_LEN);
   struct dirent *tempdir = malloc(sizeof(struct dirent));
   struct stat *stats = malloc(sizeof(struct stat));

   if (directory) {
      while ((tempdir = readdir(directory))) {
         if ((path == NULL) && checkDir(tempdir->d_name) && strcmp(tempdir->d_name, "..") != 0 &&
          strcmp(tempdir->d_name, ".") != 0)
            sprintf(curPath, "./%s", tempdir->d_name);
         else if (checkDir(tempdir->d_name) && strcmp(tempdir->d_name, "..") != 0 && 
          strcmp(tempdir->d_name, ".") != 0)
            sprintf(curPath, "%s/%s", path, tempdir->d_name);

         if (checkDir(tempdir->d_name) && strcmp(tempdir->d_name, "..") != 0 &&
          strcmp(tempdir->d_name, ".") != 0 && depth > 0) {
            sprintf(temp, "./%s", tempdir->d_name);
            chdir(tempdir->d_name);
            displayFolders(opendir("."), curPath, total, depth - 1, human);
            chdir("..");
            lstat(tempdir->d_name, stats);
            *total += roundUp((stats->st_size + (BLOCKSIZE - 1)) / BLOCKSIZE);
            if (!human)
               printf("%d\t%s\n", roundUp((int)(stats->st_size + (BLOCKSIZE - 1)) / BLOCKSIZE), curPath);
            if (human)
               printHuman(stats, -1, curPath);
         }
      }
   }
}

/*
   Used with the '-u' flag. Traverses up the directory tree n amount of times and returns after myDu has 
   finished displaying the output
*/
void goUp(int up, char *rtn) {
   char *root = "/home/eskirk";
   char *temp = malloc(sizeof(char) * MAX_LEN);
   int count = 0;

   realpath(".", rtn);
   while (count++ < up) {
      chdir("..");
      realpath(readdir(opendir("."))->d_name, temp);
      if (strcmp(root, temp) == 0)
         count = up + 1;
   }
}

/*
   Checks whether or not a given path is a file or a directory
   To be used with 
*/
int checkDir(char *path) {
   struct stat info;
   lstat(path, &info);
   return S_ISDIR(info.st_mode);
}

/*
   Prints out human-readable values for size using different units
   Used with the '-h' (human readable) flag
*/
void printHuman(struct stat *stats, long tot, char *curPath) {
   float val;

   if (stats != NULL) 
      val = (float)stats->st_size;
   else {
      val = tot;
   }

   if (val > 0 && val < ONE_K) {
      if (*curPath == '!')
         printf("%7.1f B\ttotal\n", val);
      else
         printf("%7.1f B\t%s\n", val, curPath);
   }
   else if (val > ONE_K) {
      val = (float)(val / ONE_K);
      if (val < ONE_K) {
         if (*curPath == '!')
            printf("%7.1f KB\ttotal\n", val);
         else
            printf("%7.1f KB\t%s\n", val, curPath);
      }
      else if (val > ONE_K) {
         val = (float)(val / ONE_K);
         if (val < BLOCKSIZE) {
            if (*curPath == '!')
               printf("%7.1f MB\ttotal\n", val);
            else
               printf("%7.1f MB\t%s\n", val, curPath);
         }
         else if (val > ONE_K) {
            val = (float)(val / ONE_K);
            if (*curPath == '!')
               printf("%7.1f GB\ttotal\n", val);
            else
               printf("%7.1f GB\t%s\n", val, curPath);
         }
      }
   }
}

/*
   Traverses the directory tree and only ouputs files and directories that contain the substring 'sub'
*/ 
void findSub(DIR *directory, char *path, char *sub) {
   char *temp = malloc(sizeof(char) * MAX_LEN);
   char *curPath = malloc(sizeof(char) * MAX_LEN);
   struct dirent *tempdir = malloc(sizeof(struct dirent));
   struct stat *stats = malloc(sizeof(struct stat));

   if (directory) {
      while ((tempdir = readdir(directory)) ) {
         if (path == NULL)
            sprintf(curPath, "./%s", tempdir->d_name);
         else
            sprintf(curPath, "%s/%s", path, tempdir->d_name);
         if (checkDir(tempdir->d_name) && strcmp(tempdir->d_name, "..") != 0 && 
          strcmp(tempdir->d_name, ".") != 0) {
            sprintf(temp, "./%s", tempdir->d_name);
            chdir(tempdir->d_name);
            findSub(opendir("."), curPath, sub);
            chdir("..");
         }
         if (strstr(tempdir->d_name, sub)) {
            lstat(tempdir->d_name, stats);
            printf("%d\t%s\n", (int)(stats->st_size + BLOCKSIZE - 1) / BLOCKSIZE, curPath);
         }
      }
   }
}

/*
   Rounds up the number to the next increment of four 
   To be used for calculating blocksize and other goodies
*/
int roundUp(int num) {
   int remainder = abs(num) % 4;

   if (remainder == 0)
      return num;
   else 
      return num + 4 - remainder;
}

/*
   flags:
      -a:      Displays an entry for each file in a file hierarchy
      -dn:     Goes n directories deep during recursion
      -c:      Displays a grand total
      -h:      Displays the output in a human readable way using prefixes like Byte, KB, MB...
      -f<%s>:  Tallies up only files that contain the substring s
      -v:      Displays a visual tree of the current directory
      -un:     Goes up n directories and then does what it needs to do
*/
int main(int argc, char *argv[]) {
   int all = 0, depth = 5, total = 0, human = 0, count = 1, visual = 0, look = 0, none = 0, up = 0;
   long tot = 0;
   char *find = malloc(sizeof(char) * MAX_LEN), *temp = malloc(sizeof(char) * 10), *path = malloc(sizeof(char) * MAX_LEN);
   char *rtn = malloc(sizeof(char) * MAX_LEN);
   DIR *directory = opendir(".");
   
   if (argc > 1) {
      do {
         switch(argv[count][1]) {
            case 'a':
               all = 1;
               break;
            case 'd':
               depth = atoi(&argv[count][2]);
               break;
            case 'h':
               human = 1;
               break;
            case 'f':
               strcpy(find, &argv[count][2]);
               look = 1;
               break;
            case 'c':
               total = 1;
               break;
            case 'v':
               visual = 1;
               break;
            case 'u':
               up = atoi(&argv[count][2]);
               break;
            default:
               none = 1;
               break;
         }
      } while (argv[++count] != NULL);
   }

   if (up) {
      if (up && look) {
         if (up > 2)
            up = 2;
      }
      goUp(up, rtn);
      directory = opendir(".");
   }

   if (none || (!all && !look)) {
      displayFolders(directory, NULL, &tot, depth, human);
   }

   if (all) {
      if (look) {
         printf("Look through all files on this user for substring '%s'?\ny/n: ", find);
         scanf("%s", temp);
         if (*temp == 'y' || *temp == 'Y') {
            realpath(".", path);
            chdir(getenv("HOME"));
            if (visual)
               system("tree");
            directory = opendir(".");
            findSub(directory, NULL, find);
            chdir(path);
         }
      }
      else {
         if (visual)
            system("tree");
         if (!human)
            displayAll(directory, NULL, &tot, 0, depth, human);
      }
   }

   if (visual && !all && !up) {
      system("tree");
   }

   if (look) {
      if (all) {
         directory = opendir(getenv("HOME"));
      }
      findSub(directory, NULL, find);
   }
   if (up) 
      chdir(rtn);

   if (total && !up) {
      tot = 0;
      directory = opendir(".");
      displayAll(directory, NULL, &tot, 1, depth, human);
      if (!human)
         printf("%ld\ttotal\n", tot);
      else
         printHuman(NULL, tot, "!");
   }
   else {
      tot = 0;
      directory = opendir(".");
      displayAll(directory, NULL, &tot, 1, depth, human);
   }

   if (up && visual) {
      goUp(up, rtn);
      directory = opendir(".");
      system("tree");
   }
   if (!human)
      printf("%ld\t%s\n", tot, ".");
   if (human)
      printHuman(NULL, tot, ".");

   return 0;
}
