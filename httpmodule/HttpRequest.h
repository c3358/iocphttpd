#pragma once
#include "HttpHeader.h"
//#include "HParser.h"
#include "Enums.h"
#include "HttpTemplate.h"
#include "RequestParser.h"

class HTTPMODULE_API HttpRequest : public HttpTemplate
{
public:
	HttpRequest();
	~HttpRequest();

	void Parse(char *content);
	char* GetUrl();
	MethodType GetMethod();
	const char* GetContent();

	std::vector<std::string> urlParams;

private:
	//headerparser::HParser hParser;
	HttpHeader httpHeader;
	RequestParser requestParser;
};
