#include "meme_request_handler.h"

#include <algorithm>
#include <fstream> // For ifstream.
#include <iostream>
#include <shared_mutex>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sqlite3.h>

#include "logging.h"
#include "mime.h"
#include "request_handler.h"

#define HTML_FORM_FILE "/form.html"
#define HTML_CREATE_FILE "/create.html"
#define HTML_VIEW_FILE "/view.html"
#define HTML_LIST_FILE "/list.html"
#define SQL_DATABASE_FILE "/meme.db"
#define MEME_IMAGE_URL "http://35.197.37.173/memes/memes_images/"

// Mutex for accessing meme.db on separate threads
std::shared_mutex meme_lock;

struct Meme {
  std::string top_text;
  std::string bottom_text;
  std::string image;
};

// Helpers
void init_meme_database(std::string database_name) {
  Logger *log = Logger::GetLogger();
  sqlite3 *db;

  const std::string query = "CREATE TABLE IF NOT EXISTS meme_table ("
                            "id     INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "image  TEXT,"
                            "top    TEXT,"
                            "bottom TEXT,"
                            "UNIQUE (image, top, bottom)"
                            ")";

  // Create database object
  log->LogDebug("MemeRequestHandler :: init_meme_database: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(meme_lock);
  int rc = sqlite3_open(database_name.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogFatal("MemeRequestHandler :: init_meme_database: could not create/open SQL database: " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    exit(1);
  }

  // Execute above query to create meme table
  char *error_message;
  rc = sqlite3_exec(db, query.c_str(), 0, 0, &error_message);
  if (rc != SQLITE_OK) {
    log->LogFatal("MemeRequestHandler :: init_meme_database: could not execute SQL query: " + std::string(error_message));
    sqlite3_free(error_message);
    sqlite3_close(db);
    exit(1);
  }

  log->LogInfo("MemeRequestHandler :: init_meme_database: opened SQL meme table");
  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: handle_create: unlocking thread");
  u_lock.unlock();
}

void remove_sus_characters(std::string& s) {
  std::replace(s.begin(), s.end(), '\\', '\0');
  std::replace(s.begin(), s.end(), '\'', '\0');
  std::replace(s.begin(), s.end(), '"', '\0');
  std::replace(s.begin(), s.end(), '\n', '\0');
  std::replace(s.begin(), s.end(), '\r', '\0');
}

void parse_meme_request(struct Meme& meme, const std::string& request) {
  // The prompts that precede each input
  const std::string top_text_prompt = "top_text=";
  const std::string bottom_text_prompt = "bottom_text=";
  const std::string meme_image_prompt = "image=";

  // The delimiters between each input
  int delimiter_1 = request.find("&" + bottom_text_prompt);
  int delimiter_2 = request.find("&" + meme_image_prompt, delimiter_1 + 1);

  // Grab the inputs beteween the prompts and delimiters
  std::string top_text = request.substr(top_text_prompt.size(), delimiter_1 - top_text_prompt.size());
  std::string bottom_text = request.substr(delimiter_1 + bottom_text_prompt.size() + 1, delimiter_2 - delimiter_1 - bottom_text_prompt.size() - 1);
  std::string image = request.substr(delimiter_2 + meme_image_prompt.size() + 1);

  // Replace input '+'s with spaces
  std::replace(top_text.begin(), top_text.end(), '+', ' ');
  std::replace(bottom_text.begin(), bottom_text.end(), '+', ' ');
  std::replace(image.begin(), image.end(), '+', ' ');

  // Sanitize input
  remove_sus_characters(top_text);
  remove_sus_characters(bottom_text);
  remove_sus_characters(image);

  // Update struct
  meme.top_text = top_text;
  meme.bottom_text = bottom_text;
  meme.image = image;
}

MemeRequestHandler::MemeRequestHandler(const std::string& path, NginxConfig* config)
  : RequestHandler(), location_(path) {
  Logger *log = Logger::GetLogger();

  log->LogDebug("MemeRequestHandler :: MemeRequestHandler: in constructor");
    if (config->statements_.size() < 3) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing statements in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* memes_created_stmt = config->statements_[0].get();
  if (memes_created_stmt->tokens_[0] != "memes_created_root" || memes_created_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'memes_created_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* images_stmt = config->statements_[1].get();
  if (images_stmt->tokens_[0] != "images_root" || images_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'images_root' in config");
    bad_ = true;
    return;
  }

  NginxConfigStatement* html_stmt = config->statements_[2].get();
  if (html_stmt->tokens_[0] != "html_root" || html_stmt->tokens_.size() != 2) {
    log->LogError("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + " is missing 'html_root' in config");
    bad_ = true;
    return;
  }

  memes_created_root_ = memes_created_stmt->tokens_[1];
  images_root_ = images_stmt->tokens_[1];
  html_root_ = html_stmt->tokens_[1];
  bad_ = false;
  log->LogInfo("MemeRequestHandler :: MemeRequestHandler: location_ = " + path + ", created_memes_root = " + memes_created_root_ 
                + ", images_root = " + images_root_ + ", html_root = " + html_root_);
  database_ = memes_created_root_ + SQL_DATABASE_FILE;

  // Create directory if it doesn't already exist
  if (!boost::filesystem::exists(memes_created_root_)) {
    log->LogInfo("MemeRequestHandler :: MemeRequestHandler: path " + memes_created_root_ + " does not exist, creating now");
    boost::filesystem::create_directories(memes_created_root_);
  }

  // Store all image_root_ images into a map with coresponding id number.
  int id = 0;
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator itr(images_root_); itr != end_itr; ++itr) {
    if (boost::filesystem::is_regular_file(itr->status()) && itr->path().filename() != ".DS_Store") {
      log->LogInfo("MemeRequestHandler :: MemeRequestHandler: image loaded into image_map_: ID: " + std::to_string(id) + " NAME: " + itr->path().filename().string());
      image_map_[id] = itr->path().filename().string();
      id++;
    }
  }
  log->LogInfo("MemeRequestHandler :: MemeRequestHandler: total images loaded into image_map_: " + std::to_string(image_map_.size()));

  init_meme_database(database_);
}

int MemeRequestHandler::handle_form_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  log->LogInfo("MemeRequestHandler :: handle_form_request: responding to request for a html meme form");
  
  //Set body with form html
  std::string file_path = html_root_ + HTML_FORM_FILE;
  std::string file_ext = file_path.substr(file_path.find_last_of(".") + 1);

  log->LogInfo("MemeRequestHandler :: handle_form_request: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    log->LogError("MemeRequestHandler :: handle_form_request: file path " + file_path + " is not a regular file");
    return handle_not_found(res);
  }

  std::string body((std::istreambuf_iterator<char>(istream)),
                   (std::istreambuf_iterator<char>()));

  // Insert image options into html form
  std::string sel_target = "<option>Select a template image...</option>\n";
  size_t pos = body.find(sel_target) + sel_target.length(); // Position after target string
  for (int i=0; i<image_map_.size(); i++) {
    std::string insert_str = "          <option value=" + std::to_string(i) + ">" + image_map_[i] + "</option>\n";
    body.insert(pos, insert_str);
    pos += insert_str.length();
  }
  log->LogInfo("MemeRequestHandler :: handle_form_request: HTML form body is:\n" + body);

  int content_length = body.length();
  Mime mime;
  std::string content_type = mime.getContentType(file_ext);

  res.set(http::field::content_type, content_type);
  res.set(http::field::content_length, std::to_string(content_length));

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body;

  return HTTP_STATUS_OK;
}

int MemeRequestHandler::handle_create(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  if (req.body() == "") {
    log->LogError("MemeRequestHandler :: handle_create: request was sent with an empty body");
    return handle_bad_request(res);
  }

  // Extract image, top text, and bottom text
  Meme meme;
  try {
    parse_meme_request(meme, req.body());
  } catch (std::exception& e) {
    std::string error = e.what();
    log->LogError("MemeRequestHandler :: handle_create: error parsing request body: " + error);
    return handle_bad_request(res);
  }

  log->LogInfo("MemeRequestHandler :: handle_create: creating meme with top text: '"
               + meme.top_text + "', bottom text: '"
               + meme.bottom_text + "', and image: '"
               + meme.image + "'");

  // Add meme to database
  sqlite3 *db;
  sqlite3_stmt *statement;

  const std::string insert_query = "INSERT INTO meme_table "
                                   "(image, top, bottom) "
                                   "VALUES (" + meme.image + ", \"" + meme.top_text + "\", \"" + meme.bottom_text + "\")";

  log->LogDebug("MemeRequestHandler :: handle_create: locking thread for writing");
  std::unique_lock<std::shared_mutex> u_lock(meme_lock);
  int rc = sqlite3_open(database_.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_create: could not open SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  char *error_message;
  log->LogDebug("MemeRequestHandler :: handle_create: executing SQL query: " + insert_query);
  rc = sqlite3_exec(db, insert_query.c_str(), 0, 0, &error_message);
  if (rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
    log->LogError("MemeRequestHandler :: handle_create: could not execute SQL query (errno " + std::to_string(rc) + "): " + std::string(error_message));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: handle_create: unlocking thread");
  u_lock.unlock();

  // Get meme ID number
  int meme_id = 0;
  const std::string get_query = "SELECT id FROM meme_table "
                                "WHERE image = \"" + meme.image + "\" "
                                "AND top = \"" + meme.top_text + "\" "
                                "AND bottom = \"" + meme.bottom_text + "\"";

  log->LogDebug("MemeRequestHandler :: handle_create: locking thread for writing");
  u_lock.lock();
  rc = sqlite3_open(database_.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_create: could not open SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_prepare_v2(db, get_query.c_str(), -1, &statement, NULL);

  log->LogDebug("MemeRequestHandler :: handle_create: executing SQL query: " + get_query);
  rc = sqlite3_step(statement);
  if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
    log->LogError("MemeRequestHandler :: handle_create: could not evaluate SQL query (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  meme_id = sqlite3_column_int(statement, 0);
  log->LogInfo("MemeRequestHandler :: handle_create: meme ID from SQL is: " + std::to_string(meme_id));

  rc = sqlite3_finalize(statement);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_create: could not get meme ID from SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: handle_create: unlocking thread");
  u_lock.unlock();

  // Set body with html create form
  std::string file_path = html_root_ + HTML_CREATE_FILE;
  std::string file_ext = file_path.substr(file_path.find_last_of(".") + 1);

  log->LogInfo("MemeRequestHandler :: handle_create: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    log->LogError("MemeRequestHandler :: handle_create: file path " + file_path + " is not a regular file");
    return handle_not_found(res);
  }

  std::string body((std::istreambuf_iterator<char>(istream)),
                   (std::istreambuf_iterator<char>()));

  // Insert created meme ref into create html
  std::string sel_target = "<div class=\"created-meme\">\n";
  size_t pos = body.find(sel_target) + sel_target.length(); // Position after target string
  std::string insert_str = "        Created meme! <a href=\"/meme/view?id=" + std::to_string(meme_id) + "\">" + std::to_string(meme_id) + "</a>";
  body.insert(pos, insert_str);
  log->LogInfo("MemeRequestHandler :: handle_create: HTML create body is:\n" + body);

  // Return HTTP response
  int content_length = body.length();
  const std::string content_type = "text/html";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body;

  return HTTP_STATUS_OK;
}

int MemeRequestHandler::handle_retrieve(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  std::string request_uri(req.target().begin(), req.target().end());
  size_t found = request_uri.find("?id=");
  if (req.body() == "" && found == std::string::npos) {
    log->LogError("MemeRequestHandler :: handle_retrieve: request was sent with an empty body");
    return handle_bad_request(res);
  }

  // Extract the desired id from the request
  int meme_id;
  try {
    if (found == std::string::npos) {
      meme_id = boost::lexical_cast<int>(req.body());
    } else {
      log->LogDebug("Substring: " + request_uri.substr(14));
      meme_id = boost::lexical_cast<int>(request_uri.substr(14));
    }
  } catch(const boost::bad_lexical_cast&) {
    log->LogInfo("MemeRequestHandler :: handle_retrieve: request's id is not an int");
    return handle_bad_request(res);
  }

  // Bad request if id is greater than the number of memes
  if (get_meme_count(res) == 0 || meme_id > get_meme_count(res)) {
    log->LogInfo("MemeRequestHandler :: handle_retrieve: desired meme id is greater than the number of memes created");
    return handle_bad_request(res);
  }

  // Retrieve meme via SQL
  Meme meme;
  sqlite3 *db;
  sqlite3_stmt *statement;

  const auto retrieve_query = "SELECT image, top, bottom FROM meme_table "
                              "WHERE id = \"" + std::to_string(meme_id) + "\"";

  // Use shared lock for reading
  log->LogDebug("MemeRequestHandler :: handle_retrieve: locking thread for reading");
  std::shared_lock<std::shared_mutex> s_lock(meme_lock);
  int rc = sqlite3_open(database_.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_retrieve: could not open SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_prepare_v2(db, retrieve_query.c_str(), -1, &statement, NULL);

  log->LogDebug("MemeRequestHandler :: handle_retrieve: executing SQL query: " + retrieve_query);
  rc = sqlite3_step(statement);
  if (rc == SQLITE_DONE)
  {
    log->LogError("MemeRequestHandler :: handle_retrieve: could not find meme with ID: " + std::to_string(meme_id));
    sqlite3_close(db);
    return handle_bad_request(res);
  } else if (rc != SQLITE_ROW) {
    log->LogError("MemeRequestHandler :: handle_retrieve: could not evaluate SQL query (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  // Extract string from SQL retrieval
  int image = sqlite3_column_int(statement, 0);
  std::string top_text = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 1)));
  std::string bottom_text = std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement, 2)));
  log->LogDebug("MemeRequestHandler :: handle_retrieve: returned from SQL:\n" + std::to_string(image) + "\n" + top_text + "\n" + bottom_text);

  rc = sqlite3_finalize(statement);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_retrieve: could not get meme from SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: handle_retrieve: unlocking thread");
  s_lock.unlock();

  // Return HTTP response with two texts and image
  std::string image_url = MEME_IMAGE_URL + image_map_[image];
  log->LogDebug("MemeRequestHandler :: handle_retrieve: Image URL: " + image_url);

  //Set body with view html
  std::string file_path = html_root_ + HTML_VIEW_FILE;
  std::string file_ext = file_path.substr(file_path.find_last_of(".") + 1);

  log->LogInfo("MemeRequestHandler :: handle_retrieve: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    log->LogError("MemeRequestHandler :: handle_retrieve: file path " + file_path + " is not a regular file");
    return handle_not_found(res);
  }

  std::string body((std::istreambuf_iterator<char>(istream)),
                   (std::istreambuf_iterator<char>()));

  // Insert image options into html form
  std::string sel_target = "<body>\n";
  size_t pos = body.find(sel_target) + sel_target.length(); // Position after target string
  std::string insert_str =  "<img src=\"" + image_url + "\" />\n"
                            "<h1 id=\"topText\">" + top_text + "</h1>\n"
                            "<h1 id=\"bottomText\">" + bottom_text + "</h1>\n";
  body.insert(pos, insert_str);
  log->LogInfo("MemeRequestHandler :: handle_retrieve: HTML body is:\n" + body);

  int content_length = body.length();
  const std::string content_type = "text/html";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body;

  return HTTP_STATUS_OK;
}

int MemeRequestHandler::handle_list(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  // Retrieve all meme IDs from the database
  std::vector<int> meme_ids;
  sqlite3* db;
  sqlite3_stmt* statement;

  const std::string list_query = "SELECT id FROM meme_table";

  // Use shared lock for reading
  log->LogDebug("MemeRequestHandler :: handle_list: locking thread for reading");
  std::shared_lock<std::shared_mutex> s_lock(meme_lock);
  int rc = sqlite3_open(database_.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_list: could not open SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  rc = sqlite3_prepare_v2(db, list_query.c_str(), -1, &statement, NULL);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_list: could not prepare SQL query (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  while (sqlite3_step(statement) == SQLITE_ROW) {
    int meme_id = sqlite3_column_int(statement, 0);
    meme_ids.push_back(meme_id);
  }

  rc = sqlite3_finalize(statement);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: handle_list: could not finalize SQL statement (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: handle_list: unlocking thread");
  s_lock.unlock();

  //Set body with list html
  std::string file_path = html_root_ + HTML_LIST_FILE;
  std::string file_ext = file_path.substr(file_path.find_last_of(".") + 1);

  log->LogInfo("MemeRequestHandler :: handle_list: File path used is " + file_path + "\n");
  std::ifstream istream(file_path, std::ios::in | std::ios::binary);

  if (!boost::filesystem::is_regular_file(file_path) || !istream.good()) {
    log->LogError("MemeRequestHandler :: handle_list file path " + file_path + " is not a regular file");
    return handle_not_found(res);
  }

  std::string body((std::istreambuf_iterator<char>(istream)),
                   (std::istreambuf_iterator<char>()));

  // Insert image options into html form
  std::string sel_target = "<body>\n";
  size_t pos = body.find(sel_target) + sel_target.length(); // Position after target string
  
  // Create a simple list of meme URLs using their IDs
  std::string list_html = "<header><h1>Meme List</h1></header>\n"
                          "<ul>\n";
  for (const auto& meme_id : meme_ids) {
    std::string meme_url = "/meme/view?id=" + std::to_string(meme_id);
    list_html += "<li><a href=\"" + meme_url + "\">Meme " + std::to_string(meme_id) + "</a></li>\n";
  }
  list_html += "</ul>\n";

  body.insert(pos, list_html);
  log->LogInfo("MemeRequestHandler :: handle_list: HTML body is:\n" + body);

  int content_length = body.length();
  const std::string content_type = "text/html";

  res.set(http::field::content_length, std::to_string(content_length));
  res.set(http::field::content_type, content_type);

  res.reason("OK");
  res.result(HTTP_STATUS_OK);
  res.body() = body;

  return HTTP_STATUS_OK;
}

int MemeRequestHandler::handle_request(http::request<http::string_body> req, http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();

  res.version(req.version());

  if (req.method_string() == "") {
    log->LogError("MemeRequestHandler :: handle_request: missing HTTP method");
    return handle_bad_request(res);
  }

  if (bad_) {
    log->LogError("MemeRequestHandler :: handle_request: bad config given");
    return handle_bad_request(res);
  }

  int ret_code = 0;
  std::string request_uri(req.target().begin(), req.target().end());

  if (request_uri == "/meme") {
    ret_code = handle_form_request(req, res);
  } else if (request_uri == "/meme/create") {
    ret_code = handle_create(req, res);
  } else if (boost::starts_with(request_uri, "/meme/view")) {
    // Handle viewing of created memes
    ret_code = handle_retrieve(req, res);
  } else if (request_uri == "/meme/list") {
    // Handle listing of created memes
    ret_code = handle_list(req, res);
  } else {
    log->LogInfo("MemeRequestHandler :: handle_request: encountered request for nonexisting directory");
    ret_code = handle_not_found(res);
  }

  return ret_code;
}

int MemeRequestHandler::get_meme_count(http::response<http::string_body>& res) {
  Logger* log = Logger::GetLogger();
  sqlite3 *db;
  sqlite3_stmt *statement;

  // Get meme ID number
  int meme_id = 0;
  const std::string count_query = "SELECT COUNT(*) FROM meme_table";

  // Use shared lock for reading
  log->LogDebug("MemeRequestHandler :: get_meme_count: locking thread for reading");
  std::shared_lock<std::shared_mutex> s_lock(meme_lock);
  int rc = sqlite3_open(database_.c_str(), &db);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: get_meme_count: could not open SQL database (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_prepare_v2(db, count_query.c_str(), -1, &statement, NULL);

  log->LogDebug("MemeRequestHandler :: get_meme_count: executing SQL query: " + count_query);
  rc = sqlite3_step(statement);
  if (rc == SQLITE_DONE)
  {
    log->LogError("MemeRequestHandler :: get_meme_count: could not find count of memes");
    sqlite3_close(db);
    return handle_bad_request(res);
  } else if (rc != SQLITE_ROW) {
    log->LogError("MemeRequestHandler :: get_meme_count: could not evaluate SQL query (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  // Extract string from SQL retrieval
  int meme_count = sqlite3_column_int(statement, 0);
  log->LogDebug("MemeRequestHandler :: get_meme_count: returned from SQL:\n" + std::to_string(meme_count));

  rc = sqlite3_finalize(statement);
  if (rc != SQLITE_OK) {
    log->LogError("MemeRequestHandler :: get_meme_count: could not finalize count of memes (errno " + std::to_string(rc) + "): " + std::string(sqlite3_errmsg(db)));
    sqlite3_close(db);
    return handle_internal_server_error(res);
  }

  sqlite3_close(db);
  log->LogDebug("MemeRequestHandler :: get_meme_count: unlocking thread");
  s_lock.unlock();

  return meme_count;
}
