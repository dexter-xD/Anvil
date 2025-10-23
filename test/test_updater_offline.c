#include <stdio.h>
#include <string.h>
#include <assert.h>

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

int main() {
    printf("Running offline updater tests...\n");

    assert(compare_versions("1.0.0", "1.0.1") < 0);
    assert(compare_versions("1.1.0", "1.0.9") > 0);
    assert(compare_versions("2.0.0", "1.9.9") > 0);
    assert(compare_versions("1.1.1", "1.1.1") == 0);
    assert(compare_versions("1.1.1-beta", "1.1.1") == 0);
    assert(compare_versions("1.1.0-beta", "1.1.1-beta") < 0);
    printf("✓ Version comparison tests passed\n");

    assert(strcmp(extract_version_from_tag("beta_1.1.1"), "1.1.1-beta") == 0);
    assert(strcmp(extract_version_from_tag("v1.2.0"), "1.2.0") == 0);
    assert(strcmp(extract_version_from_tag("1.3.0"), "1.3.0") == 0);
    printf("✓ Version extraction tests passed\n");

    printf("✓ All offline tests passed!\n");
    return 0;
}