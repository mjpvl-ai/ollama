#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// Function to send request to Ollama and save response
void ollama_infer(const char *model, const char *prompt) {
    CURL *curl;
    CURLcode res;
    struct ResponseData response = {NULL, 0};
    FILE *file;

    // JSON payload
    char post_data[512];
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
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response.data);
}

int main() {
    const char *model = "deepseek-llm:7b-chat";  // Change model as needed
    const char *prompt = "Explain the concept of reinforcement learning.";

    ollama_infer(model, prompt);
    return 0;
}
