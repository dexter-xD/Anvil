#include "../include/anvil.h"
#include "../include/colors.h"
#include "../include/version.h"
#include <curl/curl.h>
#include <json-c/json.h>

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
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Anvil-Updater-Test/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        printf(RED "Error: %s" RESET "\n", curl_easy_strerror(res));
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

int test_github_api_connection() {
    printf(BRIGHT_CYAN "Testing GitHub API connection..." RESET "\n");

    const char *test_url = "https://api.github.com/repos/dexter-xd/anvil/releases/latest";
    APIResponse *response = fetch_api_data(test_url);

    if (!response) {
        printf(RED "✗ Failed to connect to GitHub API" RESET "\n");
        return 0;
    }

    printf(BRIGHT_GREEN "✓ Successfully connected to GitHub API" RESET "\n");
    printf(DIM "Response size: %zu bytes" RESET "\n", response->size);

    free(response->data);
    free(response);
    return 1;
}

int test_json_parsing() {
    printf(BRIGHT_CYAN "Testing JSON parsing..." RESET "\n");

    const char *test_url = "https://api.github.com/repos/dexter-xd/anvil/releases/latest";
    APIResponse *response = fetch_api_data(test_url);

    if (!response) {
        printf(RED "✗ Failed to fetch data for JSON test" RESET "\n");
        return 0;
    }

    json_object *root = json_tokener_parse(response->data);
    if (!root) {
        printf(RED "✗ Failed to parse JSON response" RESET "\n");
        free(response->data);
        free(response);
        return 0;
    }

    json_object *tag_name_obj, *name_obj;
    if (json_object_object_get_ex(root, "tag_name", &tag_name_obj) &&
        json_object_object_get_ex(root, "name", &name_obj)) {

        const char *tag_name = json_object_get_string(tag_name_obj);
        const char *name = json_object_get_string(name_obj);

        printf(BRIGHT_GREEN "✓ Successfully parsed JSON" RESET "\n");
        printf("  Latest release: " BRIGHT_BLUE "%s" RESET "\n", name);
        printf("  Tag: " BRIGHT_BLUE "%s" RESET "\n", tag_name);

        char *version = extract_version_from_tag(tag_name);
        printf("  Extracted version: " BRIGHT_BLUE "%s" RESET "\n", version);
    } else {
        printf(RED "✗ JSON missing expected fields" RESET "\n");
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    json_object_put(root);
    free(response->data);
    free(response);
    return 1;
}

int test_version_comparison() {
    printf(BRIGHT_CYAN "Testing version comparison..." RESET "\n");

    struct {
        const char *v1;
        const char *v2;
        int expected; 
    } tests[] = {
        {"1.0.0", "1.0.1", -1},
        {"1.1.0", "1.0.9", 1},
        {"2.0.0", "1.9.9", 1},
        {"1.1.1", "1.1.1", 0},
        {"1.1.1-beta", "1.1.1", 0}, 
        {"1.1.0-beta", "1.1.1-beta", -1},
    };

    int passed = 0;
    int total = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < total; i++) {
        int result = compare_versions(tests[i].v1, tests[i].v2);
        int normalized = (result > 0) ? 1 : (result < 0) ? -1 : 0;

        if (normalized == tests[i].expected) {
            printf(BRIGHT_GREEN "  ✓" RESET " %s vs %s = %d\n", 
                   tests[i].v1, tests[i].v2, normalized);
            passed++;
        } else {
            printf(RED "  ✗" RESET " %s vs %s = %d (expected %d)\n", 
                   tests[i].v1, tests[i].v2, normalized, tests[i].expected);
        }
    }

    printf("Version comparison: " BRIGHT_BLUE "%d/%d" RESET " tests passed\n", passed, total);
    return passed == total;
}

int test_release_listing() {
    printf(BRIGHT_CYAN "Testing release listing..." RESET "\n");

    const char *test_url = "https://api.github.com/repos/dexter-xd/anvil/releases";
    APIResponse *response = fetch_api_data(test_url);

    if (!response) {
        printf(RED "✗ Failed to fetch releases" RESET "\n");
        return 0;
    }

    json_object *root = json_tokener_parse(response->data);
    if (!root) {
        printf(RED "✗ Failed to parse releases JSON" RESET "\n");
        free(response->data);
        free(response);
        return 0;
    }

    int releases_len = json_object_array_length(root);
    printf(BRIGHT_GREEN "✓ Found %d releases" RESET "\n", releases_len);

    int max_show = releases_len > 5 ? 5 : releases_len;
    for (int i = 0; i < max_show; i++) {
        json_object *release = json_object_array_get_idx(root, i);
        json_object *tag_name_obj, *name_obj, *prerelease_obj;

        if (json_object_object_get_ex(release, "tag_name", &tag_name_obj) &&
            json_object_object_get_ex(release, "name", &name_obj) &&
            json_object_object_get_ex(release, "prerelease", &prerelease_obj)) {

            const char *tag_name = json_object_get_string(tag_name_obj);
            const char *name = json_object_get_string(name_obj);
            int is_prerelease = json_object_get_boolean(prerelease_obj);

            char *version = extract_version_from_tag(tag_name);

            printf("  %d. " BRIGHT_BLUE "%s" RESET " (%s)%s\n", 
                   i + 1, version, tag_name, is_prerelease ? " [BETA]" : "");
        }
    }

    json_object_put(root);
    free(response->data);
    free(response);
    return 1;
}

int test_deb_asset_detection() {
    printf(BRIGHT_CYAN "Testing .deb asset detection..." RESET "\n");

    const char *test_url = "https://api.github.com/repos/dexter-xd/anvil/releases/latest";
    APIResponse *response = fetch_api_data(test_url);

    if (!response) {
        printf(RED "✗ Failed to fetch latest release" RESET "\n");
        return 0;
    }

    json_object *root = json_tokener_parse(response->data);
    if (!root) {
        printf(RED "✗ Failed to parse release JSON" RESET "\n");
        free(response->data);
        free(response);
        return 0;
    }

    json_object *assets_obj;
    if (!json_object_object_get_ex(root, "assets", &assets_obj)) {
        printf(RED "✗ No assets found in release" RESET "\n");
        json_object_put(root);
        free(response->data);
        free(response);
        return 0;
    }

    int assets_len = json_object_array_length(assets_obj);
    printf("Found %d assets:\n", assets_len);

    char *deb_url = NULL;
    char *deb_name = NULL;

    for (int i = 0; i < assets_len; i++) {
        json_object *asset = json_object_array_get_idx(assets_obj, i);
        json_object *name_obj, *url_obj, *size_obj;

        if (json_object_object_get_ex(asset, "name", &name_obj) &&
            json_object_object_get_ex(asset, "browser_download_url", &url_obj) &&
            json_object_object_get_ex(asset, "size", &size_obj)) {

            const char *name = json_object_get_string(name_obj);
            const char *url = json_object_get_string(url_obj);
            int size = json_object_get_int(size_obj);

            printf("  - " BRIGHT_BLUE "%s" RESET " (%d bytes)\n", name, size);

            if (strstr(name, ".deb") && strstr(name, "amd64")) {
                deb_url = strdup(url);
                deb_name = strdup(name);
                printf(BRIGHT_GREEN "    ✓ Found .deb package!" RESET "\n");
            }
        }
    }

    int success = (deb_url != NULL);
    if (success) {
        printf(BRIGHT_GREEN "✓ .deb asset detected: %s" RESET "\n", deb_name);
    } else {
        printf(RED "✗ No .deb asset found" RESET "\n");
    }

    if (deb_url) free(deb_url);
    if (deb_name) free(deb_name);
    json_object_put(root);
    free(response->data);
    free(response);

    return success;
}

int main() {
    printf(BRIGHT_CYAN "=== Anvil Updater Test Suite ===" RESET "\n\n");

    curl_global_init(CURL_GLOBAL_DEFAULT);

    int tests_passed = 0;
    int total_tests = 0;

    total_tests++; if (test_github_api_connection()) tests_passed++;
    printf("\n");

    total_tests++; if (test_json_parsing()) tests_passed++;
    printf("\n");

    total_tests++; if (test_version_comparison()) tests_passed++;
    printf("\n");

    total_tests++; if (test_release_listing()) tests_passed++;
    printf("\n");

    total_tests++; if (test_deb_asset_detection()) tests_passed++;
    printf("\n");

    printf(BRIGHT_CYAN "=== Test Results ===" RESET "\n");
    if (tests_passed == total_tests) {
        printf(BRIGHT_GREEN "✓ All tests passed! (%d/%d)" RESET "\n", tests_passed, total_tests);
        printf(BRIGHT_GREEN "Updater functionality is working correctly." RESET "\n");
    } else {
        printf(RED "✗ Some tests failed (%d/%d)" RESET "\n", tests_passed, total_tests);
        printf(YELLOW "Check network connection and GitHub API availability." RESET "\n");
    }

    curl_global_cleanup();

    return (tests_passed == total_tests) ? 0 : 1;
}