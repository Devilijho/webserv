#include "RequestHandler.hpp"

/*Sets data and the variables needed for the dynamic file handling*/

int	setData(RequestHandlerData &data, ServerConfig &dataServer, const LocationConfig* location)
{
	data.StatusLine = "HTTP/1.1 200 OK";
	setRequestBody(data);
	setQueryData(data);

	// ✅ DEBUG: Verificar POST data parsing
	std::cerr << "🔍 [DEBUG] Raw request body: '" << data.requestBody << "'" << std::endl;
	std::cerr << "🔍 [DEBUG] Request body length: " << data.requestBody.length() << std::endl;

	if (location && !location->cgi_path.empty()) {
		data.args_str.push_back(location->cgi_path);
	} else {
		data.args_str.push_back("/usr/bin/php-cgi");  // Fallback
	}
	data.args_str.push_back(data.FileName);
	data.env_str.push_back("REQUEST_METHOD=" + data.requestMethod);
	data.env_str.push_back(std::string("SCRIPT_FILENAME=") + data.FileName);
	// data.env_str.push_back(std::string("SCRIPT_FILENAME=/home/safuente/Documents/mrd/www/script/post_msg.php"));
	data.env_str.push_back("REDIRECT_STATUS=CGI");
	data.env_str.push_back("HTTP_USER_AGENT=SANTI");
	data.env_str.push_back("SERVER_PROTOCOL=HTTP/1.1");
	data.env_str.push_back("QUERY_STRING=" + data.query);
	data.env_str.push_back("MAX_FILE_SIZE=" + toString(dataServer.client_max_body_size));
	data.env_str.push_back("GATEWAY_INTERFACE=CGI/1.1");
	data.env_str.push_back("SERVER_NAME=" + dataServer.server_name);
	data.env_str.push_back("REMOTE_ADDR=localhost");
	data.env_str.push_back("SERVER_PORT=" + toString(dataServer.port));
	data.env_str.push_back("CONTENT_TYPE=" + getRequestContentType(data));
	if (data.requestMethod == "POST")
		data.env_str.push_back("CONTENT_LENGTH=" + toString(data.requestBody.length()));
	else if (data.requestMethod == "GET")
		data.env_str.push_back("CONTENT_LENGTH=0");

	for (unsigned long i = 0; i < data.args_str.size(); i++)
		data.args.push_back(const_cast<char *>(data.args_str[i].c_str()));
	for (unsigned long i = 0; i < data.env_str.size(); i++)
		data.env.push_back(const_cast<char *>(data.env_str[i].c_str()));

	data.args.push_back(NULL);
	data.env.push_back(NULL);
	data.FileContentType = getContentType(data.FileName);
	return (SUCCESS);
}

/*Finds the static file (html, css, ico), reads its content at returns it trough a pipe to
the server (not yet but easy to implement)*/


int	handle_static_request(RequestHandlerData &data)
{
	std::string buffer;
	std::ostringstream oss;

	if (data.FileName == "./www/")
		data.FileName = "./www/index.html";
	data.staticFile.open(data.FileName.c_str());
	if (data.staticFile.is_open() == false)
		return (ERROR);
	oss << data.staticFile.rdbuf();
	data.FileContent = oss.str();
	data.staticFile.close();
	return (SUCCESS);
}

/*Executes a script such as PHP with phpCGI, returns the output trough a pipe*/

int	handle_dynamic_request(RequestHandlerData &data) {
    // ✅ DEBUG: Añadir logs
    std::cerr << "🔍 [CGI] Executing: " << PATH_INFO << std::endl;
    std::cerr << "🔍 [CGI] Script: " << data.FileName << std::endl;
    std::cerr << "🔍 [CGI] POST data size: " << data.requestBody.size() << " bytes" << std::endl;

    if (pipe(data.fdOut) == ERROR || pipe(data.fdIn) == ERROR)
        return ERROR;

    pid_t pid = fork();
    int child_status = 0;
    int return_value = SUCCESS;
    char buffer;

    if (pid == -1)
        return ERROR;
    else if (pid == 0) {
        // Child process
        close(data.fdOut[0]);  // Close read end
        close(data.fdIn[1]);   // Close write end

        dup2(data.fdOut[1], STDOUT_FILENO);
        dup2(data.fdIn[0], STDIN_FILENO);

        // ✅ DEBUG: Verificar environment
        std::cerr << "🔍 [CGI-CHILD] Environment variables:" << std::endl;
        for (size_t i = 0; i < data.env_str.size(); ++i) {
            std::cerr << "  " << data.env_str[i] << std::endl;
        }

        child_status = execve(PATH_INFO, data.args.data(), data.env.data());
        std::cerr << "❌ [CGI-CHILD] execve failed: " << strerror(errno) << std::endl;
        _exit(1);  // Si llegamos aquí, execve falló
    }
    else {
        // Parent process
        close(data.fdOut[1]);  // Close write end
        close(data.fdIn[0]);   // Close read end

        // ✅ Escribir POST data ANTES de cerrar el pipe
        if (!data.requestBody.empty()) {
            std::cerr << "🔍 [CGI-PARENT] Sending POST data: " << data.requestBody << std::endl;
            ssize_t written = write(data.fdIn[1], data.requestBody.c_str(), data.requestBody.size());
            std::cerr << "🔍 [CGI-PARENT] Wrote " << written << " bytes" << std::endl;
        }
        close(data.fdIn[1]);   // Close after writing

        // Read CGI output
        data.FileContent.clear();
        while (read(data.fdOut[0], &buffer, 1) > 0) {
            data.FileContent += buffer;
        }
        close(data.fdOut[0]);

        waitpid(pid, &child_status, 0);
        return_value = WEXITSTATUS(child_status);

        // ✅ NUEVO: Separar headers CGI del body
        if (!data.FileContent.empty()) {
            size_t headers_end = data.FileContent.find("\r\n\r\n");
            if (headers_end == std::string::npos) {
                headers_end = data.FileContent.find("\n\n");
            }

            if (headers_end != std::string::npos) {
                // Separar headers y body
                std::string cgi_headers = data.FileContent.substr(0, headers_end);
                data.FileContent = data.FileContent.substr(headers_end + (cgi_headers.find("\r\n") != std::string::npos ? 4 : 2));

                // Parsear Content-Type del CGI si existe
                size_t ct_pos = cgi_headers.find("Content-type:");
                if (ct_pos == std::string::npos) ct_pos = cgi_headers.find("Content-Type:");
                if (ct_pos != std::string::npos) {
                    size_t ct_end = cgi_headers.find("\n", ct_pos);
                    std::string ct_line = cgi_headers.substr(ct_pos, ct_end - ct_pos);
                    size_t colon = ct_line.find(":");
                    if (colon != std::string::npos) {
                        data.FileContentType = ct_line.substr(colon + 1);
                        // Limpiar espacios
                        while (!data.FileContentType.empty() && data.FileContentType[0] == ' ')
                            data.FileContentType = data.FileContentType.substr(1);
                        while (!data.FileContentType.empty() &&
                               (data.FileContentType[data.FileContentType.length()-1] == '\r' ||
                                data.FileContentType[data.FileContentType.length()-1] == '\n'))
                            data.FileContentType = data.FileContentType.substr(0, data.FileContentType.length()-1);
                    }
                }
            }
        }

        // ✅ DEBUG: Log CGI output
        std::cerr << "🔍 [CGI-PARENT] CGI output size: " << data.FileContent.size() << " bytes" << std::endl;
        std::cerr << "🔍 [CGI-PARENT] CGI exit status: " << return_value << std::endl;
        if (!data.FileContent.empty()) {
            std::cerr << "🔍 [CGI-PARENT] First 200 chars: " << data.FileContent.substr(0, 200) << std::endl;
        }
    }

    return return_value;
}

/*fills some variables and returns a error page */

void errorHandling(RequestHandlerData &data,const ServerConfig &srv, int code)
{
	std::map<int, std::string>::const_iterator it = srv.error_pages.find(code);
	std::string returnData;
	if (it != srv.error_pages.end())
		data.FileName = it->second;
	else
		data.FileName = "./www/error/default.html";
	data.FileContentType = "html";
	data.StatusLine = getStatusMessage(code);
	if (access(data.FileName.c_str(), R_OK | F_OK) != 0)
		return ;
	if (handle_static_request(data) != 0)
		return ;
}

/*assembles the http response and returns it as a string */

std::string http_response(RequestHandlerData &data, ServerConfig &srv)
{
    // ✅ DEBUG: Añadir logs para ver qué se está enviando
    std::cerr << "🔍 [DEBUG] Building response for: " << data.FileName << std::endl;
    std::cerr << "🔍 [DEBUG] Content size: " << data.FileContent.size() << " bytes" << std::endl;
    std::cerr << "🔍 [DEBUG] Status: " << data.StatusLine << std::endl;

    std::string response =
    data.StatusLine + "\r\n"
    + "Connection: close\r\n"
    + "Last-Modified: " + getFileDate(data.FileName) + "\r\n"
    + "Date: " + getDate() + "\r\n"
    + "Content-Length: " + toString(data.FileContent.size()) + "\r\n";

    // ✅ NUEVO: Usar Content-Type del CGI o default inteligente
    if (data.FileContentType.find("/") != std::string::npos) {
        // CGI ya estableció Content-Type completo
        response += "Content-Type: " + data.FileContentType + "\r\n";
    } else {
        // Usar formato antiguo para archivos estáticos
        response += "Content-Type: text/" + data.FileContentType + "\r\n";
    }

    response += "Accept-Ranges: bytes\r\n"
    + std::string("ETag: ") + getETag(data.FileName) + "\r\n"  // ← ✅ CAMBIAR ESTA LÍNEA
    + "Server: " + srv.server_name + "\r\n"
    + "\r\n"
    + data.FileContent;

    // ✅ DEBUG: Log response headers
    size_t headers_end = response.find("\r\n\r\n");
    std::cerr << "📤 [DEBUG] Response headers:\n" << response.substr(0, headers_end + 4) << std::endl;
    std::cerr << "📤 [DEBUG] Total response size: " << response.size() << " bytes" << std::endl;

    return response;
}


/*handles delete requests */

void	handle_delete_request(RequestHandlerData &data)
{
    std::remove(data.FileName.c_str());
    data.StatusLine = "HTTP/1.1 204 No Content";
    data.FileContent = "";
}
