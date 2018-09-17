#ifndef USER_INI_READER_H
#define USER_INI_READER_H
#else
#error You have already included an httpserv.h and it should not be included twice
#endif /* USER_INI_READER_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct ConfigItems {
    char key[60];
    char value[60];
};

class IniReader{
private:
    struct ConfigItems item[80]; //Suport 80 items
    int itemsNum;
public:
    IniReader(){
        itemsNum = 0;
    }

    void parseIniFile(const char *fileName) {
        char buf[120]; /* Support lines up to 120 characters */
        char parts[2][60]; /* Support up to 60 characters in each part */

        FILE* fp;

        if ((fp = fopen(fileName, "r")) == NULL) {
            printf("Unable to open file: %s", fileName);
            return;
        }
        
        while (fgets(buf, sizeof(buf), fp) != NULL)
        {
            if (sscanf(buf, "%s = %s", parts[0], parts[1]) == 2) {
                strcpy(item[itemsNum].key, parts[0]);
                strcpy(item[itemsNum].value, parts[1]);
                itemsNum++;
            }
        }
        
        itemsNum--;
        fclose(fp);
    }

    char* getConfigStr(const char *key){
        //Check to see if anything got parsed?
        if (itemsNum == 0){
            return NULL;
        }
        for (int x = 0; x <= itemsNum; x++){
            if (strcmp(key, item[x].key) == 0){
                return item[x].value;
            }
        }

        return NULL;
    }
    char* getConfigStr(const char *key,char* defaultVal){
        char* res = getConfigStr(key);
        return (res == NULL)?defaultVal:res;
    }
 
    int getConfigInt(const char *key){
        char* val = getConfigStr(key);
        if (val != NULL){
            return atoi(val);
        }
        return -1;
    }
    int getConfigInt(const char *key,int defaultVal){
        int val = getConfigInt(key); 
        return (val == -1)?defaultVal:val;
    }
};

