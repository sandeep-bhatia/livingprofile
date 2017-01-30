#include "stdafx.h"
#include <stdio.h>
#include <curl/curl.h>
#include <regex>
#include <ostream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <limits>    

std::string StripHTML(std::string source)
{
	try
	{
		std::string result;
		source = std::regex_replace(source, (*new std::regex("\t")), "");
		source = std::regex_replace(source, (*new std::regex("\r")), "");
		source = std::regex_replace(source, (*new std::regex("\n")), "");
		source = std::regex_replace(source, (*new std::regex("( )+")), " ");
		source = std::regex_replace(source, (*new std::regex("<( )*head([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("(<( )*(/)( )*head( )*>)")), "");
		source = std::regex_replace(source, (*new std::regex("(<head>).*(</head>)")), "");
		source = std::regex_replace(source, (*new std::regex("<a([^>])*?.*?</a>")), "");
		source = std::regex_replace(source, (*new std::regex("< a([^>])*?.*?</a>")), "");
		source = std::regex_replace(source, (*new std::regex("<!--([^>])*?.*?-->")), "");
		source = std::regex_replace(source, (*new std::regex("<script([^>])*?.*?</script>")), "");
		source = std::regex_replace(source, (*new std::regex("<style([^>])*?.*?</style>")), "");
		source = std::regex_replace(source, (*new std::regex("(<( )*(/)( )*style( )*>)")), "");
		source = std::regex_replace(source, (*new std::regex("(<style>).*(</style>)")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*td([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*a([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*br( )*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*li( )*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*div([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*tr([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("<( )*p([^>])*>")), "");
		source = std::regex_replace(source, (*new std::regex("<[^>]*>")), "");
		source = std::regex_replace(source, (*new std::regex("&bull;")), "");
		source = std::regex_replace(source, (*new std::regex("&lsaquo;")), "<");
		source = std::regex_replace(source, (*new std::regex("&rsaquo;")), " > ");
		source = std::regex_replace(source, (*new std::regex("&trade;")), " (tm) ");
		source = std::regex_replace(source, (*new std::regex("&frasl;")), " / ");
		source = std::regex_replace(source, (*new std::regex("&lt;")), " < ");
		source = std::regex_replace(source, (*new std::regex("&gt;")), " > ");
		source = std::regex_replace(source, (*new std::regex("&copy;")), " (c) ");
		source = std::regex_replace(source, (*new std::regex("&reg;")), " (r) ");
		source = std::regex_replace(source, (*new std::regex("&(.{2,6});")), "");
		source = std::regex_replace(source, (*new std::regex("(\r)( )+(\r)")), "");
		source = std::regex_replace(source, (*new std::regex("(\t)( )+(\t)")), "");
		source = std::regex_replace(source, (*new std::regex("(\r)( )+(\r)")), "");
	}
	catch(...)
	{
	}

	return source;
}

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = (char *)malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = (char *)realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

std::string retrieveString( char* buf, int len ) {
    return std::string( buf, len );
}

int main(void)
{
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl) {
    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, "http://techcrunch.com/2013/06/24/appfuel-launch/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);
	std::string str = retrieveString(s.ptr, s.len);
	str = StripHTML(str);
    free(s.ptr);
	std::cout << str;
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}