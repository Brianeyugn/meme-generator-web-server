#include "gtest/gtest.h"
#include "api_request_handler.h"
#include <string>

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;
using http::string_body;

TEST_F(ApiHandlerTest, ApiHandlerPOSTTest)
{
    std::string req = "/api/Shoes";
    std::string rep = "";
    req_handler_crud -> handle_request(req, rep);
    EXPECT_EQ(boost::filesystem::exists( "../crud_data/Shoes/1" ), true);
    boost::filesystem::remove_all("../crud_data/Shoes");
    EXPECT_EQ(file_to_id.count("Shoes"), true);
    std::vector<int> ids = {1};
    EXPECT_EQ(ids, file_to_id["Shoes"]);
    EXPECT_EQ(rep.result(), http::status::created);
    EXPECT_EQ(std::string(rep.body().data()), "Created entry at {\"id\":1}\n");
}