# Ollama Container Inference using cURL in C

This project demonstrates how to send an inference request to the Ollama container using a C program that executes a `curl` command. The program sends a prompt to the Ollama API and saves the response in a text file.

## Prerequisites

Ensure you have the following installed on your system:

1. **Ollama Container**
   - Ollama should be running on your system. If not installed, follow the official guide:  
     [Ollama Installation](https://ollama.com)

2. **GCC Compiler**
   - Install GCC if not already installed:
     ```sh
     sudo apt update && sudo apt install gcc -y  # Ubuntu/Debian
     brew install gcc  # macOS (Homebrew)
     ```
   
3. **cURL**
   - Install `curl` if not installed:
     ```sh
     sudo apt install curl -y  # Ubuntu/Debian
     brew install curl  # macOS
     ```

