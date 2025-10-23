#include "anvil.h"
#include "colors.h"
#include "version.h"
#include <curl/curl.h>
#include <json-c/json.h>

#ifndef GITHUB_REPO
#define GITHUB_REPO "dexter-xd/anvil"
#endif

#define GITHUB_API_URL "https://api.github.com/repos/" GITHUB_REPO "/releases"
#define GITHUB_LATEST_URL "https://api.github.com/repos/" GITHUB_REPO "/releases/latest"
#define TEMP_DIR "/tmp/anvil_update"
#define INSTALL_PATH "/usr/bin/anvil"

typedef struct {
    char *data;
    size_t size;
} APIResponse;

static size_t write_callback(void *contents, size_t size, size_t nmemb, APIResponse *response) {
    size_t total_size = size * nmemb;
    char *new_data = realloc(response->data, response->size + total_size + 1);

    if (!new_data) {
        printf(RED "Error: Memory allocation failed" RESET "\n");
        return 0;
    }

    response->data = new_data;
    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = 0;

    return total_size;
}

static int download_file(const char *url, const char *output_path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        printf(RED "Error: Failed to initialize curl" RESET "\n");
        return 0;
    }

    fp = fopen(output_path, "wb");
    if (!fp) {
        printf(RED "Error: Cannot create file %s" RESET "\n", output_path);
        curl_easy_cleanup(curl);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Anvil-Updater/1.0");

    printf(CYAN "Downloading %s..." RESET "\n", url);
    res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        printf(RED "Error: Download failed: %s" RESET "\n", curl_easy_strerror(res));
        unlink(output_path);
        return 0;
    }

    return 1;
}

static APIResponse* fetch_api_data(const char *url) {
    CURL *curl;
    CURLcode res;
    APIResponse *response = malloc(sizeof(APIResponse));

    if (!response) return NULL;

    response->data = malloc(1);
    response->size = 0;

    curl = curl_easy_init();
    if (!curl) {
        free(response->data);
        free(response);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Anvil-Updater/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        free(response->data);
        free(response);
        return NULL;
    }

    return response;
}

static char* extract_version_from_tag(const char *tag_name) {
    static char version[64];

    if (strncmp(tag_name, "beta_", 5) == 0) {
        snprintf(version, sizeof(version), "%s-beta", tag_name + 5);
        return version;
    }

    if (tag_name[0] == 'v') {
        strcpy(version, tag_name + 1);
        return version;
    }

    strcpy(version, tag_name);
    return version;
}

static int compare_versions(const char *v1, const char *v2) {

    int v1_major, v1_minor, v1_patch;
    int v2_major, v2_minor, v2_patch;

    char v1_clean[64], v2_clean[64];
    strcpy(v1_clean, v1);
    strcpy(v2_clean, v2);

    char *beta1 = strstr(v1_clean, "-beta");
    char *beta2 = strstr(v2_clean, "-beta");
    if (beta1) *beta1 = 0;
    if (beta2) *beta2 = 0;

    sscanf(v1_clean, "%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2_clean, "%d.%d.%d", &v2_major, &v2_minor, &v2_patch);

    if (v1_major != v2_major) return v1_major - v2_major;
    if (v1_minor != v2_minor) return v1_minor - v2_minor;
    return v1_patch - v2_patch;
}

static int install_deb_package(const char *deb_path) {
    char command[512];

    printf(YELLOW "Installing package (requires sudo)..." RESET "\n");
    printf(DIM "Running: sudo dpkg -i %s" RESET "\n", deb_path);

    snprintf(command, sizeof(command), "sudo dpkg -i %s", deb_path);

    int result = system(command);
    if (result != 0) {
        printf(RED "Error: Package installation failed" RESET "\n");
        printf(YELLOW "You may need to run: sudo apt-get install -f" RESET "\n");
        return 0;
    }

    printf(BRIGHT_GREEN "✓ Package installed successfully!" RESET "\n");
    printf(BRIGHT_CYAN "Run 'anvil -v' to verify the new version." RESET "\n");
    return 1;
}

int update_to_latest() {
    printf(BRIGHT_CYAN "Checking for latest version..." RESET "\n");

    APIResponse *response = fetch_api_data(GITHUB_LATEST_URL);
    if (!response) {
        printf(RED "Error: Failed to fetch release information" RESET "\n");
        return 0;
    }

    json_object *root = json_tokener_parse(response->data);
    if (!root) {
        printf(RED "Error: Failed to parse API response" RESET "\n");
        free(response->data);
        free(response);
        return 0;
    }

    json_object *tag_name_obj, *assets_obj;
    if (!json_object_object_get_ex(root, "tag_name", &tag_name_obj) ||
        !json_object_object_get_ex(root, "assets", &assets_obj)) {
        printf(RED "Error: Invalid API response format" RESET "\n");
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    const char *latest_tag = json_object_get_string(tag_name_obj);
    char *latest_version = extract_version_from_tag(latest_tag);

    printf("Current version: " BRIGHT_BLUE "%s" RESET "\n", ANVIL_VERSION_STRING);
    printf("Latest version:  " BRIGHT_GREEN "%s" RESET "\n", latest_version);

    if (compare_versions(latest_version, ANVIL_VERSION_STRING) <= 0) {
        printf(BRIGHT_GREEN "You already have the latest version!" RESET "\n");
        json_object_put(root);
        free(response->data);
        free(response);
        return 1;
    }

    int assets_len = json_object_array_length(assets_obj);
    char *download_url = NULL;
    char *asset_name = NULL;

    for (int i = 0; i < assets_len; i++) {
        json_object *asset = json_object_array_get_idx(assets_obj, i);
        json_object *name_obj, *url_obj;

        if (json_object_object_get_ex(asset, "name", &name_obj) &&
            json_object_object_get_ex(asset, "browser_download_url", &url_obj)) {

            const char *name = json_object_get_string(name_obj);
            if (strstr(name, ".deb") && strstr(name, "amd64")) {
                download_url = strdup(json_object_get_string(url_obj));
                asset_name = strdup(name);
                break;
            }
        }
    }

    if (!download_url) {
        printf(RED "Error: No .deb package found in latest release" RESET "\n");
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    system("mkdir -p " TEMP_DIR);

    char deb_path[512];
    snprintf(deb_path, sizeof(deb_path), "%s/%s", TEMP_DIR, asset_name);

    int success = 0;
    if (download_file(download_url, deb_path)) {
        success = install_deb_package(deb_path);
    }

    system("rm -rf " TEMP_DIR);
    free(download_url);
    free(asset_name);
    json_object_put(root);
    free(response->data);
    free(response);

    return success;
}

int update_to_version(const char *target_version) {
    printf(BRIGHT_CYAN "Searching for version %s..." RESET "\n", target_version);

    APIResponse *response = fetch_api_data(GITHUB_API_URL);
    if (!response) {
        printf(RED "Error: Failed to fetch releases information" RESET "\n");
        return 0;
    }

    json_object *root = json_tokener_parse(response->data);
    if (!root) {
        printf(RED "Error: Failed to parse API response" RESET "\n");
        free(response->data);
        free(response);
        return 0;
    }

    int releases_len = json_object_array_length(root);
    char *download_url = NULL;
    char *asset_name = NULL;
    int found = 0;

    for (int i = 0; i < releases_len; i++) {
        json_object *release = json_object_array_get_idx(root, i);
        json_object *tag_name_obj, *assets_obj;

        if (!json_object_object_get_ex(release, "tag_name", &tag_name_obj) ||
            !json_object_object_get_ex(release, "assets", &assets_obj)) {
            continue;
        }

        const char *tag_name = json_object_get_string(tag_name_obj);
        char *version = extract_version_from_tag(tag_name);

        if (strcmp(version, target_version) == 0 || 
            (strstr(version, target_version) && strstr(version, "-beta"))) {

            found = 1;
            printf("Found version: " BRIGHT_GREEN "%s" RESET " (tag: %s)\n", version, tag_name);

            int assets_len = json_object_array_length(assets_obj);
            for (int j = 0; j < assets_len; j++) {
                json_object *asset = json_object_array_get_idx(assets_obj, j);
                json_object *name_obj, *url_obj;

                if (json_object_object_get_ex(asset, "name", &name_obj) &&
                    json_object_object_get_ex(asset, "browser_download_url", &url_obj)) {

                    const char *name = json_object_get_string(name_obj);
                    if (strstr(name, ".deb") && strstr(name, "amd64")) {
                        download_url = strdup(json_object_get_string(url_obj));
                        asset_name = strdup(name);
                        break;
                    }
                }
            }
            break;
        }
    }

    if (!found) {
        printf(RED "Error: Version %s not found" RESET "\n", target_version);
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    if (!download_url) {
        printf(RED "Error: No .deb package found for version %s" RESET "\n", target_version);
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    system("mkdir -p " TEMP_DIR);

    char deb_path[512];
    snprintf(deb_path, sizeof(deb_path), "%s/%s", TEMP_DIR, asset_name);

    int success = 0;
    if (download_file(download_url, deb_path)) {
        success = install_deb_package(deb_path);
    }

    system("rm -rf " TEMP_DIR);
    free(download_url);
    free(asset_name);
    json_object_put(root);
    free(response->data);
    free(response);

    return success;
}