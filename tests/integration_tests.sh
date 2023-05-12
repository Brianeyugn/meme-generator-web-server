#!/bin/bash

# Get the absolute project path
PROJ_PATH="$(realpath ${BASH_SOURCE:-$0} | xargs dirname | xargs dirname)"

SERVER_PATH="${PROJ_PATH}/build/bin"
SERVER_NAME="server"

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

CONFIG_PATH="${PROJ_PATH}/configuration"
CONFIG_NAME="starter.conf"

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
  INPUT="GET /static2/test.txt HTTP/1.1
Host: www.nowhere123.com
Accept: image/gif, image/jpeg, /
Accept-Language: en-us
Accept-Encoding: gzip, deflate\r\n\r\n"

  EXPECTED_OUTPUT="HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 778
Connection: keep-alive

She was dressed in rich materials--satins, and lace and silks -- all of white. Her shoes
were white. And she had a long white veil dependent from her hair, and she had bridal flowers
in her hair, but her hair was white. Some bright jewels sparkled on her neck and on her hands,
and some other jewels lay sparkling on the table. Dresses, less splendid than the dress she
wore, and half-packed trunks, were scattered about. She had not quite finished dressing, for
she had but one shoe on -- the other was on the table near her hand -- her veil was but half
arranged, her watch and chain were not put on, and some lace for her bosom lay with those
trinkets, and with her handkerchief, and gloves, and some flowers, and a Prayer-book, all
confusedly heaped about the looking-glass."

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
