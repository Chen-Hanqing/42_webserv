httpResponse    buildCGIResponse(const requestParse& req, const std::string& interpreter, const std::string& scriptPath){
    httpResponse    res;
    std::map<std::string, std::string>  headers;
    std::string body;
    if (!CGIHandler::execute(req, interpreter, scriptPath, headers, body)){
        res.setStatus(500);
        return  res;
    }
    res.setStatus(200);
    if (headers.cout("Content-Type"))
        res.setContentType(headers["Content-Type"]);
    else
        res.setContentType("text/html");
    for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it){
        if (it->first == "Content-Type")
            continue;
        if (it->first == "Status")
            continue;
        res.addHeadersValue(it->first, it->second);
    }
    if (headers.count("Status")){
        std::stringstream   ss(headers["Status"]);
        int code;
        ss >> code;
        res.setStatus(code);
    }
    res.setBody(body);
    return res;
}