#ifndef REQUESTDISPATCHER
# define REQUESTDISPATCHER

# include "httpResponse.hpp"
# include "requestParse.hpp"
# include <fstream>

/*
1. 接收 request + validate result
2. 如果 validate 不通过 → 直接 return error response
3. 根据 method 分发
4. 调用 Router（找到对应 handler）
5. 调用 MethodHandler 执行
6. 返回 response
*/

class requestDispatcher
{
    private:
        httpResponse handlerGet(const requestParse& req);
    public:
        httpResponse dispatch(const requestParse& req, ValidationResult result);
        
};

#endif
