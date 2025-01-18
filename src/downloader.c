#include "downloader.h"
#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t write_file_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    FILE *file = (FILE *)userp;
    return fwrite(contents, size, nmemb, file);
}

bool download_file(const char *url, const char *output_path) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return false;
    }

    FILE *file = fopen(output_path, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", output_path);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    CURLcode res = curl_easy_perform(curl);
    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }

    return true;
}

// Extract a tarball to a specified directory
bool extract_tarball(const char *tarball_path, const char *output_dir) {
    struct archive *a = archive_read_new();
    struct archive *ext = archive_write_disk_new();
    struct archive_entry *entry;

    archive_read_support_format_tar(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, tarball_path, 10240) != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open tarball: %s\n", archive_error_string(a));
        archive_read_free(a);
        return false;
    }

    if (archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                                                ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS) != ARCHIVE_OK) {
        fprintf(stderr, "Failed to configure extraction options.\n");
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *current_file = archive_entry_pathname(entry);
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", output_dir, current_file);
        archive_entry_set_pathname(entry, full_path);

        if (archive_write_header(ext, entry) == ARCHIVE_OK) {
            char buffer[8192];
            ssize_t size;

            while ((size = archive_read_data(a, buffer, sizeof(buffer))) > 0) {
                if (archive_write_data(ext, buffer, size) < 0) {
                    fprintf(stderr, "Error writing data for file: %s\n", current_file);
                    break;
                }
            }
        } else {
            fprintf(stderr, "Failed to write header for file: %s\n", current_file);
        }

        archive_write_finish_entry(ext);
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return true;
}


bool delete_archive(const char *tarball_path) {
    if (remove(tarball_path) != 0) {
        fprintf(stderr, "Failed to delete archive: %s\n", tarball_path);
        return false;
    }

    return true;
}