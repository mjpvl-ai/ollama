#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

#define OUTPUT_FILE "output.txt"
#define OLLAMA_URL "http://localhost:11434/api/generate"

// Structure to store response data
struct ResponseData {
    char *data;
    size_t size;
};

// Callback function for handling the response
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    struct ResponseData *resp = (struct ResponseData *)userdata;

    char *temp = realloc(resp->data, resp->size + total_size + 1);
    if (temp == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 0;
    }

    resp->data = temp;
    memcpy(&(resp->data[resp->size]), ptr, total_size);
    resp->size += total_size;
    resp->data[resp->size] = '\0';

    return total_size;
}



// Function to send request to Ollama and process response
void ollama_infer(const char *model, const char *prompt, bool print_output) {
    CURL *curl;
    CURLcode res;
    struct ResponseData response = {NULL, 0};
    FILE *file;

    // JSON payload
    char post_data[1024];
    snprintf(post_data, sizeof(post_data), "{\"model\": \"%s\", \"prompt\": \"%s\", \"stream\": false}", model, prompt);

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return;
    }

    // Set CURL options
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, OLLAMA_URL);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // Save response to file
        file = fopen(OUTPUT_FILE, "w");
        if (file) {
            fprintf(file, "%s", response.data);
            fclose(file);
            printf("Response saved to %s\n", OUTPUT_FILE);
        } else {
            fprintf(stderr, "Failed to open file for writing\n");
        }

        // Print response if enabled
        if (print_output) {
            printf("Response:\n%s\n", response.data);
        }
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response.data);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <model> <prompt> <print_output (true/false)>\n", argv[0]);
        return 1;
    }

    const char *model = argv[1];
    const char *prompt = argv[2];
    bool print_output = (strcmp(argv[3], "true") == 0);

    ollama_infer(model, prompt, print_output);
    return 0;
}
