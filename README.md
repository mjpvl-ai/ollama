# Ollama Container Inference and Sensitive Data Masking using cURL in C

This project demonstrates how to:  
✅ **Send an inference request** to the **Ollama** container using a C program that executes a `curl` command.  
✅ **Mask sensitive data** (e.g., emails, names, URLs) using **Presidio** and handle potential errors.  

---

## **🛠 Prerequisites**  

Ensure you have the following installed on your system:  

### **1️⃣ Ollama Container**  
Ollama should be running on your system. If not installed, follow the official guide:  
[Ollama Installation](https://ollama.com)  

### **2️⃣ Presidio for Sensitive Data Masking**  
We use **Presidio Analyzer and Anonymizer** to detect and mask sensitive data (e.g., emails, phone numbers, names).  

#### **Install and Run Presidio using Docker**  
```sh
# Pull Presidio Docker images
docker pull mcr.microsoft.com/presidio-analyzer
docker pull mcr.microsoft.com/presidio-anonymizer

# Start Presidio containers
docker compose up -d
```

#### **Verify that Presidio is running**  
```sh
docker ps
```
You should see `presidio-analyzer` and `presidio-anonymizer` running.  

### **3️⃣ GCC Compiler**  
Install GCC if not already installed:  
```sh
# Ubuntu/Debian
sudo apt update && sudo apt install gcc -y  

# macOS (Homebrew)
brew install gcc  
```

### **4️⃣ cURL**  
Ensure `curl` is installed:  
```sh
# Ubuntu/Debian
sudo apt install curl -y  

# macOS
brew install curl  
```

---

## **🚀 Running the C Program**  

This C program:  
✅ Sends an inference request to the Ollama API.  
✅ Processes the response and saves it to a text file.  
✅ Uses Presidio to mask sensitive information before processing.  

### **🔹 Compile the Program**  
```sh
gcc -o mask_sensitive_info mask_sensitive_info.c
```

### **🔹 Run the Program**  
```sh
./mask_sensitive_info
```

---

## **🔍 Troubleshooting**  

### **1️⃣ Presidio Not Detecting Sensitive Data**  
Run a test request manually to verify if Presidio is working:  
```sh
curl -X POST "http://127.0.0.1:5002/analyze" \
-H "Content-Type: application/json" \
-d '{"text": "My name is John Doe and my email is johndoe@example.com.", "language": "en"}'
```

If you see an error, check Presidio logs:  
```sh
docker logs presidio-presidio-analyzer-1
```

If the error mentions missing Spacy models, install them:  
```sh
docker exec -it presidio-presidio-analyzer-1 bash
python -m spacy download en_core_web_lg
exit
docker restart presidio-presidio-analyzer-1
```

### **2️⃣ Ollama API Not Responding**  
Ensure the Ollama container is running:  
```sh
docker ps | grep ollama
```

If it's not running, restart it:  
```sh
docker restart ollama-container-name
```

Then test with:  
```sh
curl http://127.0.0.1:5001/generate
```

