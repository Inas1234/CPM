#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <stdbool.h>

bool download_file(const char *url, const char *output_path);

bool extract_tarball(const char *tarball_path, const char *output_dir);

bool delete_archive(const char *tarball_path);

#endif 
