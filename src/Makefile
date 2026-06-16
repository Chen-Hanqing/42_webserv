NAME = webserv

CPP = c++
CPPFLAG = -Wall -Wextra -Werror -std=c++98

SRC = config/configparser.cpp \
	http/httpResponse.cpp http/requestDispatcher.cpp http/requestParse.cpp http/requestTool.cpp \
	http/requestDispatcherGet.cpp http/requestDispatcherPost.cpp http/requestDispatcherDelete.cpp http/requestGetter.cpp \
	network/client_connec.cpp network/runtime_server.cpp network/web_server.cpp network/webserv_connec.cpp \
	network/webserv_dispatcher.cpp network/webserv_eventloop.cpp \
	main.cpp

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



