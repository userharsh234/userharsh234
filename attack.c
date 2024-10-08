#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

#define EMAIL_FROM    "your_email@gmail.com"  // Replace with your email
#define EMAIL_TO      "sonu0007test@outlook.com"
#define SMTP_SERVER   "smtp.gmail.com"
#define SMTP_PORT     587
#define USERNAME      "your_email@gmail.com"  // Replace with your email
#define PASSWORD      "your_email_password"   // Replace with your password (use app-specific password for Gmail)

// Generate a random access key (OTAC) of length n
void generate_access_key(char *key, int n) {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    key[n] = '\0';  // Null-terminate the string
}

// Function to send email with the generated access key
void send_email_with_access_key(char *access_key) {
    CURL *curl;
    CURLcode res = CURLE_OK;

    struct curl_slist *recipients = NULL;
    curl = curl_easy_init();

    if(curl) {
        // Set SMTP server details
        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://" SMTP_SERVER ":" SMTP_PORT);

        // Enable TLS encryption
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        // Sender's email
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, EMAIL_FROM);

        // Recipient's email
        recipients = curl_slist_append(recipients, EMAIL_TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Message body
        char email_body[1024];
        snprintf(email_body, sizeof(email_body),
                 "To: %s\r\n"
                 "From: %s\r\n"
                 "Subject: Your One-Time Access Code\r\n"
                 "\r\n"
                 "Hello,\r\n\r\n"
                 "Here is your one-time access code (OTAC): %s\r\n"
                 "Please use this code to authenticate your session.\r\n\r\n"
                 "Thank you.\r\n",
                 EMAIL_TO, EMAIL_FROM, access_key);

        // Set email body
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, email_body);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        // Send the email
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
}

int main() {
    // Generate a new access key (OTAC)
    char access_key[16];  // OTAC with 15 characters
    generate_access_key(access_key, 15);

    // Send the OTAC via email
    printf("Sending OTAC to %s...\n", EMAIL_TO);
    send_email_with_access_key(access_key);
    printf("Access key sent!\n");

    return 0;
}
