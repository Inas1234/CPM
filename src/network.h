#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

typedef struct {
    char *data;    
    size_t size;   
} HttpResponse;

int http_get(const char *url, HttpResponse *response);

void free_http_response(HttpResponse *response);

#endif 
