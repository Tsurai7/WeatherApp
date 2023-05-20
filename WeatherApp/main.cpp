#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <jansson.h>

#define API_KEY "e7bb20733d20fdb40b99b096cafc44b6"
#define API_URL "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == nullptr) {
        std::cout << "Not enough memory to allocate\n";
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\0';

    return realsize;
}

int main() {

    CURL* curl_handle;
    CURLcode res;

    std::string city;
    std::cout << "Enter city:" << std::endl;
    std::cin >> city;

    char* url = new char[strlen(API_URL) + city.length() + strlen(API_KEY) + 1];
    sprintf(url, API_URL, city.c_str(), API_KEY);

    struct MemoryStruct chunk;
    chunk.memory = new char[1];
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        json_error_t error;
        json_t* root = json_loads(chunk.memory, 0, &error);
        if (!root) {
            std::cout << "Failed to parse JSON: " << error.text << std::endl;
        }
        else {
            json_t* temp = json_object_get(root, "main");
            json_t* temp_val = json_object_get(temp, "temp");
            double temp_celsius = json_real_value(temp_val) - 273.15;

            std::cout << "Temperature in " << city << " is " << temp_celsius << " degrees Celsius" << std::endl;

            json_decref(root);
        }
    }

    curl_easy_cleanup(curl_handle);
    delete[] chunk.memory;
    delete[] url;
    curl_global_cleanup();

    return 0;
}


