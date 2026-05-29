int main()
{
    std::string raw =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";

    HttpRequest req = parse(raw);
    HttpResponse res = handle(req);

    std::cout << res.toString();
}