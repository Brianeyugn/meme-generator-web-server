#!/bin/bash

# Get the absolute project path
PROJ_PATH="$(realpath ${BASH_SOURCE:-$0} | xargs dirname | xargs dirname | xargs dirname)"

SERVER_PATH="${PROJ_PATH}/build/bin"
SERVER_NAME="server"

SERVER_IP="127.0.0.1"
SERVER_PORT="8080"

CONFIG_PATH="${PROJ_PATH}/configuration"
CONFIG_NAME="starter.conf"

OUTPUT_FILE="server_response"

# Setup functions
init_server() {
  echo "Starting server"

  # This is so the config's relative paths work properly
  cd "${SERVER_PATH}/.."

  "${SERVER_PATH}/${SERVER_NAME}" "${CONFIG_PATH}/${CONFIG_NAME}" > /dev/null &
  SERVER_PID=$!

  # Go back to integration test directory
  cd ~-

  sleep 3
}

kill_server() {
  echo "Shutting down server"
  kill "${SERVER_PID}"
}

# Testing functions (0 for success)

test_error() {
  INPUT="http_requests/error_request"
  EXPECTED_OUTPUT="http_responses/expected_error_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_echo() {
  INPUT="http_requests/echo_request"
  EXPECTED_OUTPUT="http_responses/expected_echo_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_static_text() {
  INPUT="http_requests/static_text_request"
  EXPECTED_OUTPUT="http_responses/expected_static_text_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_static_image() {
  INPUT="http_requests/static_image_request"
  EXPECTED_OUTPUT="http_responses/expected_static_image_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_static_etc() {
  INPUT="http_requests/static_etc_request"
  EXPECTED_OUTPUT="http_responses/expected_static_etc_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_api_create() {
  INPUT="http_requests/api_create_request"
  EXPECTED_OUTPUT="http_responses/expected_api_create_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_api_read() {
  INPUT="http_requests/api_read_request"
  EXPECTED_OUTPUT="http_responses/expected_api_read_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_api_update() {
  INPUT="http_requests/api_update_request"
  EXPECTED_OUTPUT="http_responses/expected_api_update_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_api_list() {
  INPUT="http_requests/api_list_request"
  EXPECTED_OUTPUT="http_responses/expected_api_list_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_api_delete() {
  INPUT="http_requests/api_delete_request"
  EXPECTED_OUTPUT="http_responses/expected_api_delete_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

test_multithreading() {
  INPUT_1="http_requests/echo_request"
  INPUT_2="http_requests/health_request"

  # Send first request with a delay between each line (should take a few seconds)
  time_before=$(date +%s)
  cat "${INPUT_1}" | nc -i 1 -q 1 "${SERVER_IP}" "${SERVER_PORT}" > /dev/null &

  # Send second request (should be immediate)
  cat "${INPUT_2}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > /dev/null
  time_after=$(date +%s)

  # Check time difference with a tolerance of 3 seconds
  if [ $(expr ${time_after} - ${time_before}) -le 3 ]; then
    return 0
  else
    return 1
  fi
}

test_health() {
  INPUT="http_requests/health_request"
  EXPECTED_OUTPUT="http_responses/expected_health_response"

  cat "${INPUT}" | nc -q 1 "${SERVER_IP}" "${SERVER_PORT}" > "${OUTPUT_FILE}"
  diff "${EXPECTED_OUTPUT}" "${OUTPUT_FILE}"

  status=$?

  rm "${OUTPUT_FILE}"

  return $status
}

main() {
  # Populate array with tests we want to run (one test per line)
  test_functions=(
    test_error
    test_echo
    test_static_text
    test_static_image
    test_static_etc
    test_api_create
    test_api_read
    test_api_update
    test_api_list
    test_api_delete
    test_multithreading
    test_health
  )

  # Start the server
  init_server

  # Run tests
  declare -i passed=0
  for test in "${test_functions[@]}"
  do
    echo -e "Running test:\t${test}"
    ${test}

    # Consider a return value of 0 a success, anything else is a failure
    if [ $? -eq 0 ]; then
      echo -e "Test passed:\t${test}"
      let "passed++"
    else
      echo -e "Test failed:\t**${test}"
    fi

  done

  # Print results
  echo "${passed}/${#test_functions[@]} tests passed"

  # Stop the server
  kill_server

  # Remove testing logs
  rm -rf "${PROJ_PATH}/tests/logs"
}

main
