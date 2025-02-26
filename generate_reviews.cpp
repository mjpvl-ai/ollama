#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <curl/curl.h>

// Function to store API response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to send request to Ollama
std::string ollama_infer(const std::string& model, const std::string& prompt) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "http://localhost:11434/api/generate";
        std::string json_data = "{\"model\": \"" + model + "\", \"prompt\": \"" + prompt + "\"}";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

// Function to read CSV file
std::vector<std::pair<std::string, std::string>> read_csv(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> products;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return products;
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string product_name, description;
        std::getline(ss, product_name, ',');
        std::getline(ss, description, ',');

        if (!product_name.empty() && !description.empty()) {
            products.push_back({product_name, description});
        }
    }
    file.close();
    return products;
}

// Function to create PowerPoint slides
void create_ppt(const std::vector<std::pair<std::string, std::string>>& reviews) {
    std::ofstream ppt_file("reviews.pptx");

    if (!ppt_file.is_open()) {
        std::cerr << "Error: Could not create PowerPoint file!" << std::endl;
        return;
    }

    for (size_t i = 0; i < reviews.size() && i < 10; ++i) {
        ppt_file << "Slide " << i + 1 << "\n";
        ppt_file << "Title: " << reviews[i].first << "\n";
        ppt_file << "Review: " << reviews[i].second << "\n\n";
    }

    ppt_file.close();
    std::cout << "PowerPoint slides saved as 'reviews.pptx'!" << std::endl;
}

int main() {
    std::string model = "deepseek-llm:7b-chat"; // Change model as needed
    std::string csv_filename = "products.csv";

    // Read products from CSV
    auto products = read_csv(csv_filename);

    if (products.empty()) {
        std::cerr << "No valid products found in CSV!" << std::endl;
        return 1;
    }

    std::vector<std::pair<std::string, std::string>> reviews;

    // Generate reviews using Ollama
    for (const auto& product : products) {
        std::string prompt = "Write a detailed review for the product: " + product.first + " - " + product.second;
        std::string review = ollama_infer(model, prompt);
        reviews.push_back({product.first, review});
    }

    // Create PowerPoint slides
    create_ppt(reviews);

    return 0;
}
