


#pragma once


#include <string>
#include <map>

class Request
{
	private:
		std::string method;
		std::string path;
		std::string http_version;
		std::map<std::string, std::string> headers;
		std::string body;

		void parse(const std::string& raw_request);

	public:
		Request(const std::string& raw_request);

		std::string getMethod() const { return method; }
		std::string getPath() const { return path; }
		std::string getHttpVersion() const { return http_version; }
		std::string getHeader(const std::string& key) const;
		std::string getBody() const { return body; }
};
