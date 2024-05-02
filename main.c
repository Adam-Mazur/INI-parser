#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;
    char *value;
} key_value;

typedef struct {
    char * name;
    key_value * content;
    int length;
} section;

enum operation {plus, minus, multiply, divide};

char * readLine(FILE *fp) {
    int offset = 0;
    int bufsize = 4;
    char *buf;
    int c;

    buf = malloc(bufsize);

    while (c = fgetc(fp), c != '\n' && c != EOF) {
        if (offset == bufsize - 1) {
            bufsize *= 2;

            char *new_buf = realloc(buf, bufsize);

            buf = new_buf; 
        }

        buf[offset++] = c;
    }

    if (c == EOF && offset == 0) {
        free(buf);
        return NULL;
    }

    if (offset < bufsize - 1) {
        char *new_buf = realloc(buf, offset + 1);
        buf = new_buf;
    }

    buf[offset] = '\0';

    return buf;
}

int isValid(char c) {
    if (isalnum(c) || c == '-') return 1;
    return 0;
}

int searchTree(char * query, section * tree, int treeL, int * a, int * b) {
    char * querySection = NULL;
    char * queryKey     = NULL;
    int querySectionL   = 0;
    int queryKeyL       = 0;
    int i = 0;
    char c;
    while (c = query[i++], c != '\0') {
        if (!isValid(c) && c != '.') {
            printf("main.c: invalid query: Invalid character appeared in the query\n");
            return 1;
        }
        if (c == '.') {
            querySection = realloc(querySection, ++querySectionL);
            querySection[querySectionL-1] = '\0';
        } else if (querySection == NULL || querySection[querySectionL-1] != '\0') {
            querySection = realloc(querySection, ++querySectionL);
            querySection[querySectionL-1] = c;
        } else {
            queryKey = realloc(queryKey, ++queryKeyL);
            queryKey[queryKeyL-1] = c;
        }
    }
    if (queryKey == NULL || querySection == NULL) {
        printf("main.c: invalid query: Missing information in the query\n");
        return 1;
    } else {
        queryKey = realloc(queryKey, ++queryKeyL);
        queryKey[queryKeyL-1] = '\0';        
    }
    // Finding the value
    int foundSection = 0;
    int foundKey = 0;
    for(i = 0; i < treeL; i++) {
        if (strcmp(tree[i].name, querySection) == 0) {
            foundSection = 1;
            for (int j = 0; j < tree[i].length; j++) {
                if (strcmp(tree[i].content[j].key, queryKey) == 0) {
                    foundKey = 1;
                    *a = i;
                    *b = j;
                }
            }
        } 
    }
    if (!foundSection) {
        printf("main.c: section not found: Failed to find section: \"%s\" in the .ini file\n", querySection);
        return 1;
    }
    if (!foundKey) {
        printf("main.c: key not found: Failed to find key: \"%s\" in section: \"%s\"\n", queryKey, querySection);
        return 1;
    }
    free(querySection);
    free(queryKey);
    return 0;
}

int main(int argc, char ** argv) {
    char * path  = argv[1];
    FILE * file  = fopen(path, "r");
    
    // Parsing the INI file
    section * parsedFile = NULL;
    int parsedFileL = 0;

    char * s;
    while (s = readLine(file), s != NULL) {
        char c;
        int i = 0;
        int isFirstNonWs   = 1;
        int isSection      = 0;
        int isKeyValue     = 0;
        char * sectionName = NULL;
        char * keyName     = NULL;
        char * valueName   = NULL;
        int sectionNameL   = 0;
        int keyNameL       = 0;
        int valueNameL     = 0;
        while (c = s[i++], c != '\0') {
            // Skip whitespace characters
            if (isspace(c)) continue;
            // Skip on comments
            if (c == ';') break;
            /*
            1. Check for many [] pairs are there
            2. If there is exatly one pair, parse it as a section
            3. If there are more, throw error
            4. If there is none, check wherther there are two keywords sperated with =
            */
            if (isFirstNonWs) {
                if (c == '[') {
                    isSection = 1;
                } else if (isValid(c)) {
                    isKeyValue = 1;
                    keyName = realloc(keyName, ++keyNameL);
                    keyName[keyNameL-1] = c;
                } else {
                    printf("main.c: invalid file: Invalid character found at line: \"%s\"\n", s);
                    return 1;
                }
            } else if (isValid(c)) {
                if (isSection && (sectionName == NULL || sectionName[sectionNameL-1] != '\0')) {
                    sectionName = realloc(sectionName, ++sectionNameL);
                    sectionName[sectionNameL-1] = c;
                } else if (isKeyValue && keyName[keyNameL-1] != '\0') {
                    keyName = realloc(keyName, ++keyNameL);
                    keyName[keyNameL-1] = c;                    
                } else if (isKeyValue){
                    valueName = realloc(valueName, ++valueNameL);
                    valueName[valueNameL-1] = c;
                } else {
                    printf("main.c: invalid file: Failed to parse the file due to the line: \"%s\"\n", s);
                    return 1;                       
                }
            } else if (c == '=' && isKeyValue) {
                keyName = realloc(keyName, ++keyNameL);
                keyName[keyNameL-1] = '\0';
            } else if (c == ']' && isSection && sectionName[sectionNameL-1] != '\0') {
                sectionName = realloc(sectionName, ++sectionNameL);
                sectionName[sectionNameL-1] = '\0';                
            } else {
                printf("main.c: invalid file: Invalid character found at line: \"%s\"\n", s);
                return 1;                
            }

            isFirstNonWs = 0;
        }
        // Missing closing ']'  
        if (isSection && sectionName[sectionNameL-1] != '\0') {
            printf("main.c: invalid file: Closing ']' is missing at line: \"%s\"\n", s);
            return 1;  
        }
        // In key-value pair, there is no value
        if (isKeyValue && valueName == NULL) {
            printf("main.c: invalid file: The value in the key-value pair is missing at line: \"%s\"\n", s);
            return 1; 
        // Append '\0' to valueName    
        } else if (isKeyValue){
            valueName = realloc(valueName, ++valueNameL);
            valueName[valueNameL-1] = '\0';            
        }
        // Adding nodes to the file tree
        if (isSection) {
            parsedFile = realloc(parsedFile, (++parsedFileL)*sizeof(section));
            parsedFile[parsedFileL-1].content = NULL;
            parsedFile[parsedFileL-1].length = 0;
            parsedFile[parsedFileL-1].name = sectionName;
        } else if (isKeyValue && parsedFileL > 0) {
            parsedFile[parsedFileL-1].content 
                    = realloc(parsedFile[parsedFileL-1].content, (++parsedFile[parsedFileL-1].length)*sizeof(key_value));
            parsedFile[parsedFileL-1].content[parsedFile[parsedFileL-1].length-1].key = keyName;
            parsedFile[parsedFileL-1].content[parsedFile[parsedFileL-1].length-1].value = valueName;
        } else if (isKeyValue){
            printf("main.c: invalid file: Key-value pair appeared before the section at line: \"%s\"\n", s);
            return 1;
        }
        free(s);
    }
    // If we have an expression
    if (strcmp(argv[2], "expression") == 0) {
        char * query1 = NULL;
        char * query2 = NULL;
        int query1L   = 0;
        int query2L   = 0;
        char c;
        int i = 0;
        enum operation op = -1;
        // Parse the expression
        while (c = argv[3][i++], c != '\0') {
            if (isspace(c)) {
                if (query1 != NULL) {
                    query1 = realloc(query1, ++query1L);
                    query1[query1L-1] = '\0';
                } 
            } else if (c == '+' && op == -1) {
                op = plus;
            } else if (c == '-' && op == -1 && query1 != NULL && query1[query1L-1] == '\0') {
                op = minus;
            } else if (c == '*' && op == -1) {
                op = multiply;
            } else if (c == '/' && op == -1) {
                op = divide;
            } else if ((isValid(c) || c == '.') && (query1 == NULL || query1[query1L-1] != '\0')) {
                query1 = realloc(query1, ++query1L);
                query1[query1L-1] = c;
            } else if ((isValid(c) || c == '.') && query1[query1L-1] == '\0') {
                query2 = realloc(query2, ++query2L);
                query2[query2L-1] = c;
            } else {
                printf("main.c: invalid query: Invalid character appeared in the query\n");
                return 1;
            }
        }
        if (query1 == NULL || query2 == NULL) {
            printf("main.c: invalid query: Missing information in the query\n");
            return 1;
        } else if (query1[query1L-1] != '\0') {
            printf("main.c: invalid query: Missing the second operand in the query\n");
            return 1;
        } else if (op == -1) {
            printf("main.c: invalid query: Missing the operator in the query\n");
            return 1;
        } else {
            query2 = realloc(query2, ++query2L);
            query2[query2L-1] = '\0';            
        }
        int a1, b1, a2, b2;
        int rt = searchTree(query1, parsedFile, parsedFileL, &a1, &b1) 
                + searchTree(query2, parsedFile, parsedFileL, &a2, &b2);
        if (rt != 0) return 1;
        free(query1);
        free(query2);
        char * value1 = parsedFile[a1].content[b1].value;
        char * value2 = parsedFile[a2].content[b2].value;
        int isNum1 = 1;
        int isNum2 = 1;
        i = 0;
        // Check if values are strings or numbers
        while (c = value1[i++], c != '\0') {
            if (!isdigit(c)) {
                isNum1 = 0;
            }
        }
        i = 0;
        while (c = value2[i++], c != '\0') {
            if (!isdigit(c)) {
                isNum2 = 0;
            }
        }
        if (isNum1 && isNum2) {
            char * trash;
            int num1 = strtol(value1, &trash, 10);
            int num2 = strtol(value2, &trash, 10);
            switch (op) {
                case plus:
                    printf("%d\n", num1 + num2);
                    break;
                case minus:
                    printf("%d\n", num1 - num2);
                    break;
                case multiply:
                    printf("%d\n", num1 * num2);
                    break;
                case divide:
                    printf("%f\n", ((float)num1/(float)num2));
                    break;
            }
        } else if (!isNum1 && !isNum2) {
            if (op == plus) {
                printf("%s%s\n", value1, value2);
            } else {
                printf("main.c: invalid query: Illegal operation on strings\n");
                return 1;
            }
        } else {
            printf("main.c: invalid query: The operand types are not matching\n");
            return 1;
        }
    } else {
        int a;
        int b;
        int rt = searchTree(argv[2], parsedFile, parsedFileL, &a, &b);
        if (rt != 0) return 1;
        printf("%s\n", parsedFile[a].content[b].value);
    }
    // Cleaning up
    for (int i = 0; i < parsedFileL; i++) {
        for (int j = 0; j < parsedFile[i].length; j++) {
            free(parsedFile[i].content[j].key);
            free(parsedFile[i].content[j].value);
        }
        free(parsedFile[i].content);
        free(parsedFile[i].name);
    }
    free(parsedFile);
    fclose(file);
    return 0;
}