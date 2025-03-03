#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

#define MAX_TEXT_LEN 1024
#define MAX_JSON_LEN 4096

// Structure for response data
struct ResponseData {
    char data[MAX_JSON_LEN];
    size_t size;
};

// Callback function to store API response
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    struct ResponseData *response = (struct ResponseData *)userdata;
    size_t total_size = size * nmemb;

    if (response->size + total_size < MAX_JSON_LEN - 1) {
        strncat(response->data, ptr, total_size);
        response->size += total_size;
    }
    return total_size;
}

// Function to mask sensitive information
void mask_sensitive_info(char *input, char *json_response) {
    printf("Received JSON Response: %s\n", json_response); // Debugging line

    char output[MAX_TEXT_LEN];
    strcpy(output, input);  // Copy original text

    json_t *root;
    json_error_t error;

    root = json_loads(json_response, 0, &error);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return;
    }

    size_t index;
    json_t *entity;
    json_array_foreach(root, index, entity) {
        int start = json_integer_value(json_object_get(entity, "start"));
        int end = json_integer_value(json_object_get(entity, "end"));

        if (start < 0 || end > strlen(output) || start >= end) {
            continue; // Ignore invalid entries
        }

        // Replace sensitive text with '*'
        for (int i = start; i < end; i++) {
            output[i] = '*';
        }
    }

    json_decref(root); // Free JSON object
    printf("Sanitized Prompt: %s\n", output);
}

int main() {
    CURL *curl;
    CURLcode res;
    struct ResponseData response = {{0}, 0};

    // User input
    char input_text[MAX_TEXT_LEN] = "My name is John Doe and my email is johndoe@example.com.";

    // JSON Payload
    char json_data[MAX_JSON_LEN];
    snprintf(json_data, sizeof(json_data),
        "{ \"text\": \"%s\", \"language\": \"en\" }", input_text);

    // Initialize cURL
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "cURL initialization failed!\n");
        return 1;
    }

    // Set cURL options
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5002/analyze");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    } else {
        // Mask sensitive data
        mask_sensitive_info(input_text, response.data);
    }

    // Cleanup
    curl_easy_cleanup(curl);
    return 0;
}
