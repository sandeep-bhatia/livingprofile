// dekode.engine.client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>

typedef int (*EXTRACT)(const char*);
int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE fileModule = LoadLibrary(L"C:\\apps\\livingprofile\\engine\\core\\core\\Release\\dekode.engine.dll");
	FARPROC procAddress = GetProcAddress(fileModule, "InitializeModel");
	if(procAddress != NULL)
	{
		procAddress();
	}

	std::string text = *(new std::string("Keyword extraction is an important technique for document retrieval, Web page re-trieval, document clustering, summarization, text mining, and so on. By extracting appropriate keywords, we can easily choose which document to read to learn therelationship among documents. A popular algorithm for indexing is the tdf mea-sure, which extracts keywords that appear frequently in a document, but that don't appear frequently in the remainder of the corpus. The term keyword extraction is used in the context of text mining, for example 15. A comparable research topic is called automatic term recognition in the context of computational linguistics and automatic indexing or automatic keyword extraction in information retrieval research."));
	std::cout << std::endl << "the extracted text" << std::endl;
	std::cout << "_______________________________________________________________________________________________________" << std::endl;
	std::cout << text << std::endl;
	std::cout << "_______________________________________________________________________________________________________" << std::endl;
	EXTRACT extract = (EXTRACT)GetProcAddress(fileModule, "ExtractKeywords");

	int retVal = extract(text.c_str());
	
	return 0;
}

