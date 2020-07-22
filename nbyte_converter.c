#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include "nbyte_converter.h"

int convert_from_NBO(int n){
    if(sizeof(n) == 2){
        return ntohs(n);
    }else{
        return ntohl(n);
    }
}

int convert_to_NBO(int n) {
    if (sizeof(n) == 2) {
        return  htons(n);
    } else {
        return  htonl(n);
    }
}

