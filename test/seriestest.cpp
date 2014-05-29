#include "../async/async.hpp"

#define BOOST_TEST_MODULE SeriesTest
#include <boost/test/included/unit_test.hpp>
#include "test.hpp"

// TODO: this signature will compile with or without reference!
void task1(async::TaskCallback<int> &callback) { callback(async::OK, 1); }
void task2(async::TaskCallback<int> &callback) { callback(async::OK, 2); }
void task3(async::TaskCallback<int> &callback) { callback(async::OK, 3); }
void task_fail(async::TaskCallback<int> &callback) { callback(async::FAIL, -1); }

BEGIN_SEQUENCER_TEST(test1) {
  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { 1, 2, 3 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::OK);
  };

  std::vector<async::Task<int>> tasks = { task1, task2, task3 };
  async::series<int>(tasks, completion_callback);

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test2) {
  async::TaskCallback<int> deferred_callback;

  auto task_deferred_initiate = [&deferred_callback](async::TaskCallback<int> &callback) {
    deferred_callback = callback;
  };

  auto task_deferred_complete = [&deferred_callback]() {
    deferred_callback(async::OK, 99);
  };

  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { 1, 2, 99, 3 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::OK);
  };

  std::vector<async::Task<int>> tasks = { task1, task2, task_deferred_initiate, task3 };
  async::series<int>(tasks, completion_callback);

  // Here we're doing some other work until we're ready to invoke the callback.

  // Invoke the callback.
  task_deferred_complete();

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test3) {
  async::TaskCallback<int> deferred_callback;

  auto task_deferred_initiate = [&deferred_callback](async::TaskCallback<int> &callback) mutable {
    deferred_callback = callback;
  };

  auto task_deferred_complete = [&deferred_callback]() {
    deferred_callback(async::OK, 99);
  };

  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { 99, 99, 99 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::OK);
  };

  std::vector<async::Task<int>> tasks = { task_deferred_initiate, task_deferred_initiate, task_deferred_initiate };
  async::series<int>(tasks, completion_callback);

  // Here we're doing some other work until we're ready to invoke the callback.

  // Invoke the callback.  Invoke it three times, once per deferred task.  Note that the
  // second and third tasks run in between.
  task_deferred_complete();
  task_deferred_complete();
  task_deferred_complete();

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test4) {
  async::TaskCallback<int> deferred_callback;

  auto task_deferred_initiate = [&deferred_callback](async::TaskCallback<int> &callback) mutable {
    deferred_callback = callback;
  };

  auto task_deferred_complete = [&deferred_callback]() {
    deferred_callback(async::OK, 99);
  };

  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { 99, 1, 99 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::OK);
  };

  std::vector<async::Task<int>> tasks = { task_deferred_initiate, task1, task_deferred_initiate };
  async::series<int>(tasks, completion_callback);

  // Here we're doing some other work until we're ready to invoke the callback.

  // Invoke the callback.  Invoke it twice, once per deferred task.  Note that the
  // second and third tasks run in between.
  task_deferred_complete();
  task_deferred_complete();

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test_error_code) {
  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { 1, 2, -1 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::FAIL);
  };

  std::vector<async::Task<int>> tasks = { task1, task2, task_fail, task3 };
  async::series<int>(tasks, completion_callback);

  END_SEQUENCER_TEST();
}

BEGIN_SEQUENCER_TEST(test_error_code2) {
  auto completion_callback = [](async::ErrorCode error, std::vector<int> &results) {
    std::vector<int> expected { -1 };
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(results), end(results), begin(expected), end(expected));
    BOOST_CHECK_EQUAL(error, async::FAIL);
  };

  std::vector<async::Task<int>> tasks = { task_fail };
  async::series<int>(tasks, completion_callback);

  END_SEQUENCER_TEST();
}

BOOST_AUTO_TEST_CASE(series_test) {
}
