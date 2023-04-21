#!/bin/bash

# Get the absolute project path
PROJ_PATH="$(realpath ${BASH_SOURCE:-$0} | xargs dirname | xargs dirname)"

SERVER_PATH="${PROJ_PATH}/build/bin"
SERVER_NAME="server"

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

CONFIG_PATH="${PROJ_PATH}/configuration"
CONFIG_NAME="starter_config"

# Setup functions
init_server() {
  echo "Starting server"
  "${SERVER_PATH}/${SERVER_NAME}" "${CONFIG_PATH}/${CONFIG_NAME}" &
  sleep 3
}

kill_server() {
  echo "Shutting down server"
  kill %1
}

# Testing functions

# Give server a valid HTTP/1.1 request and expect the same message echoed back
test_echo() {
  INPUT="GET /docs/index.html HTTP/1.1
Host: www.nowhere123.com
Accept: image/gif, image/jpeg, /
Accept-Language: en-us
Accept-Encoding: gzip, deflate\r\n\r\n"

  EXPECTED_OUTPUT="HTTP/1.1 200 OK
Content-Type: text/plain

GET /docs/index.html HTTP/1.1
Host: www.nowhere123.com
Accept: image/gif, image/jpeg, /
Accept-Language: en-us
Accept-Encoding: gzip, deflate"

  echo -e "Sending:\n${INPUT}"
  echo "----------"

  output="$(echo -e "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" | tr -d '\r')"

  echo -e "Received:\n${output}"
  echo "----------"
  echo -e "Expected:\n${EXPECTED_OUTPUT}"
  echo "----------"

  if [ "${output}" = "${EXPECTED_OUTPUT}" ]; then
    return 0
  fi

  return 1
}


main() {
  # Populate array with tests we want to run (one test per line)
  test_functions=(
    test_echo
  )

  # Start the server
  init_server

  # Run tests
  declare -i passed=0
  for test in "${test_functions[@]}"
  do
    echo "Running test: ${test}"
    ${test}

    # Consider a return value of 0 a success, anything else is a failure
    if [ $? -eq 0 ]; then
      echo "Test passed: ${test}"
      let "passed++"
    else
      echo "Test failed: ${test}"
    fi

  done

  # Print results
  echo "${passed}/${#test_functions[@]} tests passed"

  # Stop the server
  kill_server
}

main
