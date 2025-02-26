#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <cstdlib>

// Structure to hold product details
struct Product {
    std::string name;
    std::string description;
    std::string image_url;
    std::string mrp;
    std::string current_price;
};

// Callback function to collect data from Curl
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Call the Ollama API to generate a review for a given prompt
std::string ollama_infer(const std::string& model, const std::string& prompt) {
    CURL* curl;
    CURLcode res;
    std::string response;
    
    curl = curl_easy_init();
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

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return response;
}

// Read product data from CSV file
std::vector<Product> read_csv(const std::string& filename) {
    std::vector<Product> products;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return products;
    }
    
    // Skip header line
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        Product product;
        std::getline(ss, product.name, ',');
        std::getline(ss, product.description, ',');
        std::getline(ss, product.image_url, ',');
        std::getline(ss, product.mrp, ',');
        std::getline(ss, product.current_price, ',');
        products.push_back(product);
    }
    file.close();
    return products;
}

// Download image using a system call (using curl)
void download_image(const std::string& url, const std::string& filename) {
    std::string command = "curl -o " + filename + " " + url;
    std::system(command.c_str());
}

// Convert text to speech and save as an MP3 file using espeak and ffmpeg
void text_to_speech(const std::string& text, const std::string& filename) {
    std::string command = "espeak \"" + text + "\" --stdout | ffmpeg -i - -ar 44100 -ac 2 -b:a 192k " + filename;
    std::system(command.c_str());
}

// Create a slide deck file (slides.txt) that will be converted to a PPTX
// The first slide includes an impressive thumbnail with image, MRP, and current price.
void create_slides_file(const std::vector<Product>& products, const std::vector<std::string>& reviews) {
    std::ofstream pptxFile("slides.txt");
    if (!pptxFile) {
        std::cerr << "Error creating slides.txt" << std::endl;
        return;
    }
    
    // First slide: Product overview (for the first product)
    pptxFile << "Slide 1: Product Overview\n";
    pptxFile << "Product: " << products[0].name << "\n";
    pptxFile << "MRP: " << products[0].mrp << "\n";
    pptxFile << "Current Price: " << products[0].current_price << "\n";
    pptxFile << "Thumbnail: product_thumbnail.jpg\n\n";
    
    // Additional slides: One per product review
    for (size_t i = 0; i < products.size(); i++) {
        pptxFile << "Slide " << (i + 2) << ": " << products[i].name << "\n";
        pptxFile << reviews[i] << "\n\n";
    }
    pptxFile.close();
}

// Convert the slides.txt file to a PPTX using LibreOffice command-line conversion
void convert_to_pptx() {
    std::string command = "libreoffice --headless --convert-to pptx slides.txt";
    std::system(command.c_str());
}

// Convert the PPTX file to a video (e.g., MP4)
void convert_pptx_to_video() {
    std::string command = "libreoffice --headless --convert-to mp4 slides.pptx";
    std::system(command.c_str());
}

int main() {
    std::string model = "deepseek-llm:7b-chat"; // Change this to your desired model
    std::string csv_file = "products.csv";      // CSV file with product details
    
    // Read product details from CSV
    std::vector<Product> products = read_csv(csv_file);
    if (products.empty()) {
        std::cerr << "No product data found. Exiting." << std::endl;
        return 1;
    }
    
    std::vector<std::string> reviews;
    
    // Generate a review for each product using Ollama
    for (size_t i = 0; i < products.size(); i++) {
        std::string prompt = "Write a professional review for the following product description:\n" + products[i].description;
        std::string response = ollama_infer(model, prompt);
        reviews.push_back(response);
        
        // Convert the review to audio (one audio file per product slide; note first slide uses image)
        std::string audio_filename = "slide" + std::to_string(i + 2) + ".mp3";
        text_to_speech(response, audio_filename);
    }
    
    // Download the image for the first product and save as thumbnail
    download_image(products[0].image_url, "product_thumbnail.jpg");
    
    // Create slides file from product details and reviews
    create_slides_file(products, reviews);
    
    // Convert slides.txt to a PowerPoint file (PPTX)
    convert_to_pptx();
    
    // Convert the PPTX file to a video output (e.g., MP4)
    convert_pptx_to_video();
    
    std::cout << "Process complete. PPTX and video have been generated." << std::endl;
    return 0;
}
