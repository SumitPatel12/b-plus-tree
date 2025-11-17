#include "btree.h"
#include <cassert>
#include <iostream>

void test_empty_array() {
  int arr[10] = {};
  auto result = find_index_greater_than_or_equal(arr, 0, 5);
  assert(result.index == 0);
  assert(result.is_duplicate == false);
}

void test_insert_at_beginning() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 1);
  assert(result.index == 0);
  assert(result.is_duplicate == false);
}

void test_insert_at_end() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 12);
  assert(result.index == 5);
  assert(result.is_duplicate == false);
}

void test_matches_last_element() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 9);
  assert(result.index == 4);
  assert(result.is_duplicate == false);
}

void test_insert_in_middle() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 5);
  assert(result.index == 2);
  assert(result.is_duplicate == false);
}

void test_duplicate_at_beginning() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 2);
  assert(result.index == 0);
  assert(result.is_duplicate == true);
}

void test_duplicate_in_middle() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 6);
  assert(result.index == 2);
  assert(result.is_duplicate == true);
}

void test_duplicate_at_end() {
  int arr[10] = {2, 4, 6, 8, 10};
  auto result = find_index_greater_than_or_equal(arr, 5, 10);
  assert(result.index == 4);
  assert(result.is_duplicate == true);
}

void test_single_element_match() {
  int arr[10] = {5};
  auto result = find_index_greater_than_or_equal(arr, 1, 5);
  assert(result.index == 0);
  assert(result.is_duplicate == true);
}

void test_single_element_less() {
  int arr[10] = {5};
  auto result = find_index_greater_than_or_equal(arr, 1, 3);
  assert(result.index == 0);
  assert(result.is_duplicate == false);
}

void test_single_element_greater() {
  int arr[10] = {5};
  auto result = find_index_greater_than_or_equal(arr, 1, 7);
  assert(result.index == 1);
  assert(result.is_duplicate == false);
}

int main() {
  test_empty_array();
  test_insert_at_beginning();
  test_insert_at_end();
  test_insert_in_middle();
  test_duplicate_at_beginning();
  test_duplicate_in_middle();
  test_duplicate_at_end();
  test_single_element_match();
  test_single_element_less();
  test_single_element_greater();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}
