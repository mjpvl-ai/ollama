#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json.h>
#include "pugixml.hpp"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to send request to Ollama API
std::string ollama_infer(const std::string& model, const std::string& prompt) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "http://localhost:11434/api/generate";
        std::string json_data = "{\"model\": \"" + model + "\", \"prompt\": \"" + prompt + "\", \"stream\": false}";

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
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

// Function to read CSV
std::vector<std::pair<std::string, std::string>> readCSV(const std::string& filename) {
    std::vector<std::pair<std::string, std::string>> products;
    std::ifstream file(filename);
    std::string line, name, description;

    if (!file.is_open()) {
        std::cerr << "Error opening CSV file!" << std::endl;
        return products;
    }

    while (getline(file, line)) {
        std::stringstream ss(line);
        if (getline(ss, name, ',') && getline(ss, description, ',')) {
            products.push_back({name, description});
        } else {
            std::cerr << "Skipping malformed line in CSV: " << line << std::endl;
        }
    }
    file.close();
    return products;
}

// Function to generate PowerPoint slides using XML (pugixml)
void generatePPTX(const std::string& pptxFile, const std::vector<std::pair<std::string, std::string>>& reviews) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("pptx");

    for (const auto& review : reviews) {
        pugi::xml_node slide = root.append_child("slide");
        slide.append_child("title").text().set(review.first.c_str());
        slide.append_child("content").text().set(review.second.c_str());
    }

    // Save as an XML-based PPTX file
    if (!doc.save_file(pptxFile.c_str())) {
        std::cerr << "Error: Failed to save PPTX file!" << std::endl;
    }
}

int main() {
    std::string model = "deepseek-llm:7b-chat";
    std::string csvFile = "products.csv";
    std::string jsonOutputFile = "reviews.json";
    std::string pptxFile = "product_reviews.xml"; // Change this to a valid PowerPoint format if needed

    std::vector<std::pair<std::string, std::string>> products = readCSV(csvFile);
    Json::Value jsonRoot;
    std::vector<std::pair<std::string, std::string>> reviews;

    for (const auto& product : products) {
        std::string prompt = "Write a detailed review for the following product: " + product.second;
        std::string response = ollama_infer(model, prompt);

        Json::Value jsonResponse;
        Json::CharReaderBuilder reader;
        std::string errs;

        std::istringstream s(response);
        if (Json::parseFromStream(reader, s, &jsonResponse, &errs)) {
            if (jsonResponse.isMember("response")) {
                std::string review = jsonResponse["response"].asString();
                jsonRoot[product.first] = review;
                reviews.push_back({product.first, review});
            } else {
                std::cerr << "Unexpected API response format: " << response << std::endl;
            }
        } else {
            std::cerr << "Failed to parse JSON response: " << errs << std::endl;
        }
    }

    // Save JSON output
    std::ofstream jsonFile(jsonOutputFile);
    jsonFile << jsonRoot.toStyledString();
    jsonFile.close();

    // Generate PowerPoint
    generatePPTX(pptxFile, reviews);

    std::cout << "Reviews saved to " << jsonOutputFile << " and slides generated in " << pptxFile << std::endl;
    return 0;
}
