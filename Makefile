NAME = webserv

CPP = c++
CPPFLAG = -Wall -Wextra -Werror -std=c++98

SRC = src/config/configparser.cpp \
	src/http/httpResponse.cpp \
	src/http/requestDispatcher.cpp \
	src/http/requestParse.cpp \
	src/http/requestTool.cpp \
	src/http/requestDispatcherGet.cpp \
	src/http/requestDispatcherPost.cpp \
	src/http/requestDispatcherDelete.cpp \
	src/http/requestGetter.cpp \
	src/network/client_connec.cpp \
	src/network/runtime_server.cpp \
	src/network/web_server.cpp \
	src/network/webserv_connec.cpp \
	src/network/webserv_dispatcher.cpp \
	src/network/webserv_eventloop.cpp \
	src/cgi/CGIEnvironment.cpp \
	src/cgi/CGIHandler.cpp \
	src/cgi/CGIResponse.cpp \
	src/cgi/CGIProcess.cpp \
	src/main.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(CPPFLAG) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CPP) $(CPPFLAG) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re



