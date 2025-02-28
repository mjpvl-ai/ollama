#include <stdio.h>
#include <string.h>
#include <regex.h>

// List of regex patterns to identify sensitive information
const char *patterns[][2] = {
    { "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}", "EMAIL_REDACTED" },  // Email
    { "[0-9]{10,15}", "PHONE_NUMBER_REDACTED" },  // Phone Numbers
    { "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+", "IP_ADDRESS_REDACTED" },  // IP Address
    { "sk-[A-Za-z0-9]{32,}", "API_KEY_REDACTED" },  // OpenAI API Key Format
    { "[A-Za-z0-9]{20,40}", "ACCESS_TOKEN_REDACTED" },  // Generic Access Token
    { "[0-9]{16}", "CREDIT_CARD_REDACTED" },  // Credit Card Numbers (16 Digits)
    { "[0-9]{3,4}", "CVV_REDACTED" },  // CVV Codes (3-4 Digits)
    { "(?i)(Google|Microsoft|Amazon|OpenAI)", "COMPANY_NAME_REDACTED" },  // Company Names
    { "(?i)(root|admin|password|pass123|securepassword)", "PASSWORD_REDACTED" },  // Passwords
    { "-----BEGIN PRIVATE KEY-----([\\s\\S]*?)-----END PRIVATE KEY-----", "PRIVATE_KEY_REDACTED" },  // Private Keys
    { "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\\.[A-Za-z0-9_-]+\\.[A-Za-z0-9_-]+", "JWT_TOKEN_REDACTED" },  // JWT Tokens
    { "[0-9]{4}-[0-9]{2}-[0-9]{2}", "DATE_REDACTED" },  // Dates (YYYY-MM-DD)
    { "[A-Z0-9]{10,20}", "IDENTIFIER_REDACTED" }  // Generic Identifiers (e.g., Employee ID, Tax ID)
};

// Function to replace sensitive info with placeholders
void mask_sensitive_info(const char *input, char *output) {
    char temp[2048];  // Buffer for modified text
    strncpy(temp, input, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';  // Ensure null-termination

    regex_t regex;
    regmatch_t match;
    int i;

    for (i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
        if (regcomp(&regex, patterns[i][0], REG_EXTENDED) == 0) {
            while (regexec(&regex, temp, 1, &match, 0) == 0) {
                int start = match.rm_so;
                int end = match.rm_eo;

                char masked[2048];
                snprintf(masked, sizeof(masked), "%.*s%s%s", start, temp, patterns[i][1], temp + end);
                strncpy(temp, masked, sizeof(temp) - 1);
                temp[sizeof(temp) - 1] = '\0';
            }
            regfree(&regex);
        }
    }

    strncpy(output, temp, sizeof(temp) - 1);
    output[sizeof(temp) - 1] = '\0';
}

// Main function to demonstrate the functionality
int main() {
    const char *user_prompt = "Hello, my name is John Doe. My email is johndoe@example.com, and my phone number is 9876543210. "
                              "I work at Google. Here is my API key: sk-abcdef1234567890. My IP is 192.168.1.1. "
                              "Use my credit card 4111111111111111 with CVV 123. JWT: eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9. "
                              "Server password: securepassword";

    char sanitized_prompt[2048];

    // Mask sensitive information
    mask_sensitive_info(user_prompt, sanitized_prompt);

    // Print the masked prompt
    printf("Sanitized Prompt: \n%s\n", sanitized_prompt);

    return 0;
}
