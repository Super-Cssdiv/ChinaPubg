#pragma once

#include <curl/curl.h>
#include <curl/easy.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>


using namespace std;


size_t receive_data(void *contents, size_t size, size_t nmemb, void *stream) {
    string *str = (string *) stream;
    (*str).append((char *) contents, size * nmemb);
    return size * nmemb;
}

string http_get_curl(const std::string &strUrl) {
    std::string result;
    CURL *pCURL = curl_easy_init();
    if (pCURL == NULL) {
        return result;
    }
    string response_body;
    curl_easy_setopt(pCURL, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(pCURL, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(pCURL, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(pCURL, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(pCURL, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(pCURL, CURLOPT_LOW_SPEED_TIME,  6);
    curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, receive_data);
    curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void *) &response_body);
    CURLcode ret = curl_easy_perform(pCURL);
    if (ret == CURLE_OK) {
        result = response_body;
    }
    curl_easy_cleanup(pCURL);
    return result;
}

string http_post_json(const std::string &strUrl, const std::string &json) {
    std::string result;
    CURL *pCURL = curl_easy_init();
    if (pCURL == NULL) {
        return result;
    }
    string response_body;
    curl_easy_setopt(pCURL, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(pCURL, CURLOPT_POST, 1);
    curl_easy_setopt(pCURL, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(pCURL, CURLOPT_CONNECTTIMEOUT, 5);
    curl_easy_setopt(pCURL, CURLOPT_TIMEOUT, 10);
    curl_easy_setopt(pCURL, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(pCURL, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(pCURL, CURLOPT_LOW_SPEED_TIME,  6);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(pCURL, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, receive_data);
    curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, (void *) &response_body);
    curl_easy_setopt(pCURL, CURLOPT_POSTFIELDS, json.c_str());
    CURLcode ret = curl_easy_perform(pCURL);
    if (ret == CURLE_OK) {
        result = response_body;
    }
    curl_easy_cleanup(pCURL);
    return result;
}