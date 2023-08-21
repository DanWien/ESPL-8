#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  char debug_mode;
  char display_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

struct fun_desc {
char *name;
void (*fun)(state* s);
};

static char* hex_formats[] = {"%hhX\n", "%hX\n", "No such unit", "%X\n"};
static char* dec_formats[] = {"%hhd\n", "%hd\n", "No such unit", "%d\n"};

void Toggle_Debug_Mode(state* s){
    if(s->debug_mode == 'n'){
        s->debug_mode = 'f';
        printf("Debug flag now off\n");
    }
    else{
        s->debug_mode = 'n';
        printf("Debug flag now on\n");
    }
}

void Set_File_Name(state* s){
    printf("Insert a new file name:\n");
    char fileName[100];
    scanf("%s",fileName);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
    strcpy(s->file_name, fileName); 
    if(s->debug_mode == 'n')
        printf("Debug: file name set to %s\n", s->file_name);
}

void Set_Unit_Size(state* s){
    int size;
    printf("Insert unit size:\n");
    scanf("%d", &size);
    if(size == 1 || size == 2 || size == 4){
        if(s->debug_mode == 'n')
            printf("Debug: set size to %d\n", size);
        s->unit_size = size;
    }
    else
        printf("invalid size\n");
}

void Load_Into_Memory(state* s){
    if (strcmp(s->file_name , "") == 0){
        printf("Empty file name\n");
        return;
    }
    FILE* file = fopen(s->file_name, "r+");
    if(file == NULL){
        printf("Error: unable to open file\n");
        return;
    }
    printf("Please enter <location> <length>: \n");
    int length = 0;
    int location = 0;
    char buffer[50]; 
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%x %d", &location, &length);
    if(s->debug_mode == 'n'){
        printf("file name: %s\n",s-> file_name);
        printf("location: %d\n",location);
        printf("length: %d\n",length);
    }
    fseek(file, location, SEEK_SET);
    s->mem_count = s->unit_size * length; // Check this later
    fread(s->mem_buf, s->unit_size, length, file);
    printf("Loaded %d units into memory\n", length);
    fclose(file);
}

void Toggle_Display_Mode(state* s){
     if(s->display_mode == 'd'){
        s->display_mode = 'h';
        printf("Display flag now on, hexadecimal representation\n");
    } else{
        s->display_mode = 'd';
        printf("Display flag now off, decimal representation\n");
    }
} 

void Memory_Display(state* s){
    printf("Please enter <address> <length>: \n");
    unsigned int address = 0;
    unsigned int length = 0;
    char input[100]; 
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %u", &address, &length);
    
    unsigned char* buffer;
    if(address == 0)
        buffer = s->mem_buf;
    else
        buffer = (unsigned char*)address;

    int u = s->unit_size;

    if(s->display_mode == 'd'){
        printf("Decimal\n=======\n");
        for(int i = 0; i < length; i++){
            int value = 0;
            memcpy(&value, buffer + i * u, u);
            printf(dec_formats[u-1], value);
        }
    } else { // display_mode is 'h'
        printf("Hexadecimal\n===========\n");
        for(int i = 0; i < length; i++){
            int value = 0;
            memcpy(&value, buffer + i * u, u);
            printf(hex_formats[u-1], value);
        }
    }
}

 

void Save_Into_File(state* s) {
   if (strcmp("", s->file_name) == 0){
        printf("Empty file name\n");
        return;
    }
    FILE* file = fopen(s->file_name, "r+");
    if(file == NULL){
        printf("Error: unable to open file\n");
        return;
    }
    int source_address = 0;
    int length = 0;
    int target_location = 0;
    printf("Please enter <source-address> <target-location> <length>\n");
    scanf("%x %x %d", &source_address, &target_location, &length);
    fseek(file, 0L, SEEK_END);
    if (target_location > ftell(file)){
        printf("target-location is greater than the size of the file\n");
        return;
    }
    fseek(file, 0, SEEK_SET);
    fseek(file, target_location, SEEK_SET);
    if (source_address != 0)
        fwrite(&source_address, s->unit_size, length, file);
    else
        fwrite(&s->mem_buf, s->unit_size, length, file);
    if(s->debug_mode == 'n')
        printf("Debug: wrote %u units from address %#010x to file %s at offset %#010x\n", length, source_address, s->file_name, target_location);
    fclose(file);
}


void Memory_Modify(state* s){
    printf("Please enter <location> <value>\n");
    int location = 0;
    int val = 0;
    scanf("%x %x", &location, &val);
    if(location >= s->mem_count){
        if(s->debug_mode == 'n')
            printf("location is beyond memory buffer\n");
        return;
    }
    if (s->debug_mode == 'n'){
        printf("location: %x\n value: %x\n",location, val);
    }
    memcpy(&s->mem_buf[location],&val,s->unit_size);
}

void quit(state* s){
    if(s->debug_mode=='n')
        printf( "quitting\n");
    exit(0);

} 

struct fun_desc menu[] ={
    {"Toggle Debug Mode", Toggle_Debug_Mode},
    {"Set File Name", Set_File_Name},
    {"Set Unit Size", Set_Unit_Size},
    {"Load Into Memory", Load_Into_Memory},
    {"Toggle Display Mode", Toggle_Display_Mode},
    {"Memory Display", Memory_Display},
    {"Save Into File", Save_Into_File},
    {"Memory Modify", Memory_Modify},
    {"Quit", quit},
    {NULL, NULL}
};

int display = 0;


int main(int argc, char **argv){
    state* s = calloc (1 , sizeof(state));
    s->unit_size = 1;
    s->debug_mode='f';
    s->display_mode='d';
    int menu_len=9;
    int num;
    while(1){
        if(s->debug_mode == 'n'){
            printf("unit_size: %d\n", s->unit_size);
            printf("file_name: %s\n", s->file_name);
            printf("mem_count: %d\n", s->mem_count);
        }

        printf("Select operation from the following menu:\n");
        for(int i=0; i<menu_len; i++){
            printf("%d-%s\n",i,menu[i].name);
        }
        printf("Option: ");
        scanf("%d" , &num);
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        if((num>=0) & (num<=menu_len)){
            printf("Within bounds\n");
            menu[num].fun(s);
        }
        else{
            printf("Not within bounds\n");
            exit(1);
        }

    }
    free(s);
    return 0;
}