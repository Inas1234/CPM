#include "network.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    HttpResponse *response = (HttpResponse *)userp;

    char *ptr = realloc(response->data, response->size + total_size + 1);
    if (!ptr) {
        return 0; 
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = '\0';

    return total_size;
}

int http_get(const char *url, HttpResponse *response) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        return 1; 
    }

    response->data = malloc(1); 
    if (!response->data) {
        curl_easy_cleanup(curl);
        return 2; 
    }
    response->data[0] = '\0';
    response->size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response->data);
        response->data = NULL;
        response->size = 0;
        curl_easy_cleanup(curl);
        return 3; 
    }

    curl_easy_cleanup(curl);
    return 0; 
}

void free_http_response(HttpResponse *response) {
    if (response->data) {
        free(response->data);
        response->data = NULL;
    }
    response->size = 0;
}
