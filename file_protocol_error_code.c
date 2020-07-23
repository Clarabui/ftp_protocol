#include  <stdio.h>      /* printf()  */

void display_error(int error_code){
    if (error_code == -1){
        printf("ERROR CODE -1 : File does not existi\n");
    }else if (error_code == -2){
        printf("ERROR CODE -2 : No read permission\n");
    }else if (error_code == -3){
        printf("ERROR CODE -3 : Cannot create or open file\n");
    }
}
