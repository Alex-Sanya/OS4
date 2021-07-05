#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>
#define MAX_FILE_PATH_NAME 4096
#define FLOAT_SIZE 4
#define BOOL_SIZE 1
#define STRDUP_ERROR -10
#define MFSTEMP_ERROR -11
#define UNLINK_ERROR -12
#define FILE_ERROR -13
#define MMAP_ERROR -14
#define FORK_ERROR -15
#define ZERO_DIVISION_ERROR -16
struct Data_{
 float res;
 bool zeroDivision;
};
typedef struct Data_ Data;
const int mapSize = FLOAT_SIZE + BOOL_SIZE;
int getTmpFile() {
 char* tmpFileName = strdup("/tmp/Lab4OSTempFile.XXXXXX");
 if (tmpFileName == NULL){
 perror("strdup error");
 exit(STRDUP_ERROR);
 }
 int fd = mkstemp(tmpFileName);
 if (fd == -1){
 perror("mfstemp error");
 exit(MFSTEMP_ERROR);
 }
 // delete
 if (unlink(tmpFileName) == -1){
 perror("unlink error");
 exit(UNLINK_ERROR);
 }
 free(tmpFileName);
 write(fd, "\0\0\0\0\0", mapSize);
 return fd;
}
void writeToMap(Data* data, void* map) {
 float* res = (float*) map;
 bool* zeroDivision = (bool*) (map + FLOAT_SIZE);
 *res = data->res;
 *zeroDivision = data->zeroDivision;
}
void readFromMap(Data* data, void* map){
 float* res = (float*) map;
 bool* zeroDivision = (bool*) (map + FLOAT_SIZE);
 data->res = *res;
 data->zeroDivision = *zeroDivision;
}
int main() {
 char pathName[MAX_FILE_PATH_NAME];
 scanf("%s",pathName);
 int fileDesc = open(pathName,O_RDONLY);
 if (fileDesc == -1) {
 perror("input file error");
 return FILE_ERROR;
 }
 int tempFileDesc = getTmpFile();
 void* map = mmap(NULL,mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, tempFileDesc,0);
 if (map == MAP_FAILED) {
 perror("mmap error");
 exit(MMAP_ERROR);
 }
 pid_t pid = fork();
 if (pid < 0) {
 perror("fork error");
 return FORK_ERROR;
 }
 // child
 else if (pid == 0) {
 Data data;
 data.res = -1;
 data.zeroDivision = false;
 dup2(fileDesc, STDIN_FILENO);
 float firstNum;
 if (scanf("%f",&firstNum) == -1) {
 perror("input file is empty\n");
 }
 else {
 data.res = firstNum;
 }
 float curNum;
 while(scanf("%f",&curNum) != EOF) {
 if (curNum == 0){
 perror("zero division error");
 data.zeroDivision = true;
 break;
 }
 else {
 data.res /= curNum;
 }
 }
 writeToMap(&data, map);
 }
 // parent
 else {
 wait(&pid);
 Data data;
 readFromMap(&data, map);
 if (data.zeroDivision){
 exit(ZERO_DIVISION_ERROR);
 }
 if (data.res != -1){
 printf("res is %f\n", data.res);
 }
 }
 close(fileDesc);
 close(tempFileDesc);
 return 0;
}
