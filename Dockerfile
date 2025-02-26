FROM ubuntu:22.04

# Install necessary libraries
RUN apt update && apt install -y curl g++ make libcurl4-openssl-dev libreoffice

# Set working directory
WORKDIR /app

# Copy C++ source file and CSV file
COPY generate_reviews.cpp .
COPY products.csv .

# Compile the C++ file
RUN g++ generate_reviews.cpp -o generate_reviews -lcurl

# Ensure the binary is executable
RUN chmod +x generate_reviews

# Run the compiled program
CMD ["./generate_reviews"]
