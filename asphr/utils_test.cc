#include "utils.hpp"

#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(SubmatrixExtraction, ExtractSingleRow) {
  const size_t db_row_length_in_bits = 64;
  const size_t number_of_rows = 8;

  // fill the db with 0s
  vector<byte> db(db_row_length_in_bits / 8 * number_of_rows, byte(0));

  const size_t subm_top_left_corner_in_bits = 4;
  const size_t subm_row_length_in_bits = 18;

  // the all 0s byte
  const auto all_0s_byte = byte(0);
  // the all 1s byte
  const auto all_1s_byte = byte(0xff);

  // db is a vector with alternating 0 and 1 bytes
  for (size_t i = 0; i < db.size(); i++) {
    if (i % 2 == 0) {
      db[i] = all_0s_byte;
    } else {
      db[i] = all_1s_byte;
    }
  }

  // extract the submatrix
  const size_t subm_rows = 1;
  const auto subm = get_submatrix_as_uint64s(
      db, db_row_length_in_bits, subm_top_left_corner_in_bits,
      subm_row_length_in_bits, subm_rows);

  // check that the submatrix is correct
  EXPECT_EQ(subm.size(), subm_rows);

  // check that each row is 0x3FC0
  for (size_t i = 0; i < subm.size(); i++) {
    EXPECT_EQ(subm[i], 0x3FC0);
  }
}

TEST(SubmatrixExtraction, ExtractSingleRow2) {
  const size_t db_row_length_in_bits = 64;
  const size_t number_of_rows = 8;

  // fill the db with 0s
  vector<byte> db(db_row_length_in_bits / 8 * number_of_rows, byte(0));

  size_t subm_top_left_corner_in_bits = 4;
  const size_t subm_row_length_in_bits = 18;

  // the all 0s byte
  const auto all_0s_byte = byte(0);

  // the 0b01010101 byte
  auto checkable_byte = byte(0x55);

  // db is a vector with the checkable byte followed by 0s
  for (size_t i = 0; i < db.size(); i++) {
    if (i == 0) {
      db[i] = checkable_byte;
    } else {
      db[i] = all_0s_byte;
    }
  }

  // extract the submatrix
  const size_t subm_rows = 1;
  const auto subm = get_submatrix_as_uint64s(
      db, db_row_length_in_bits, subm_top_left_corner_in_bits,
      subm_row_length_in_bits, subm_rows);

  // check that the submatrix is correct
  EXPECT_EQ(subm.size(), subm_rows);

  // check that each row is 0x14000
  for (size_t i = 0; i < subm.size(); i++) {
    std::cout << "subm[" << i << "] = " << std::bitset<64>(subm[i]).to_string()
              << std::endl;
    EXPECT_EQ(subm[i], 0x14000);
  }

  // test left_corner = 6
  subm_top_left_corner_in_bits = 5;

  // extract the submatrix
  const auto subm2 = get_submatrix_as_uint64s(
      db, db_row_length_in_bits, subm_top_left_corner_in_bits,
      subm_row_length_in_bits, subm_rows);

  // check that the submatrix is correct
  EXPECT_EQ(subm2.size(), subm_rows);

  // check that each row is 0x28000
  for (size_t i = 0; i < subm2.size(); i++) {
    std::cout << "subm2[" << i
              << "] = " << std::bitset<64>(subm2[i]).to_string() << std::endl;
    EXPECT_EQ(subm2[i], 0x28000);
  }
}

TEST(SubmatrixExtraction, ExtractSmallMatrix) {
  const size_t db_row_length_in_bits = 64;
  const size_t number_of_rows = 8;

  // fill the db with 0s
  vector<byte> db(db_row_length_in_bits / 8 * number_of_rows, byte(0));

  const size_t subm_top_left_corner_in_bits = 4;
  const size_t subm_row_length_in_bits = 18;

  // the all 0s byte
  const auto all_0s_byte = byte(0);
  // the all 1s byte
  const auto all_1s_byte = byte(0xff);

  // db is a vector with alternating 0 and 1 bytes
  for (size_t i = 0; i < db.size(); i++) {
    if (i % 2 == 0) {
      db[i] = all_0s_byte;
    } else {
      db[i] = all_1s_byte;
    }
  }

  // extract the submatrix for different number of rows.
  for (size_t subm_rows = 2; subm_rows < 7; subm_rows++) {
    // extract the submatrix
    const auto subm = get_submatrix_as_uint64s(
        db, db_row_length_in_bits, subm_top_left_corner_in_bits,
        subm_row_length_in_bits, subm_rows);

    // check that the submatrix is correct
    EXPECT_EQ(subm.size(), subm_rows);

    // check that each row is 0x3FC0
    for (size_t i = 0; i < subm.size(); i++) {
      EXPECT_EQ(subm[i], 0x3FC0);
    }
  }
}

TEST(SubmatrixExtraction, ExcessRows) {
  const size_t db_row_length_in_bits = 64;
  const size_t number_of_rows = 8;

  // fill the db with 0s
  vector<byte> db(db_row_length_in_bits / 8 * number_of_rows, byte(0));

  const size_t subm_top_left_corner_in_bits = 4;
  const size_t subm_row_length_in_bits = 18;

  // the all 0s byte
  const auto all_0s_byte = byte(0);
  // the all 1s byte
  const auto all_1s_byte = byte(0xff);

  // db is a vector with alternating 0 and 1 bytes
  for (size_t i = 0; i < db.size(); i++) {
    if (i % 2 == 0) {
      db[i] = all_0s_byte;
    } else {
      db[i] = all_1s_byte;
    }
  }

  // extract the submatrix for different number of rows.
  for (size_t subm_rows = 2; subm_rows < 7; subm_rows++) {
    // extract the submatrix
    const auto subm = get_submatrix_as_uint64s(
        db, db_row_length_in_bits, subm_top_left_corner_in_bits,
        subm_row_length_in_bits, subm_rows);

    // check that the submatrix is correct
    EXPECT_EQ(subm.size(), subm_rows);

    // check that each row is 0x3FC0
    for (size_t i = 0; i < subm.size(); i++) {
      EXPECT_EQ(subm[i], 0x3FC0);
    }
  }

  for (size_t subm_rows = 9; subm_rows < 100; subm_rows++) {
    // extract the submatrix
    const auto subm = get_submatrix_as_uint64s(
        db, db_row_length_in_bits, subm_top_left_corner_in_bits,
        subm_row_length_in_bits, subm_rows);

    // check that the submatrix is correct
    EXPECT_EQ(subm.size(), subm_rows);

    // check that each row is 0x3FC0
    for (size_t i = 0; i < 8; i++) {
      EXPECT_EQ(subm[i], 0x3FC0);
    }

    for (size_t i = 8; i < subm.size(); i++) {
      EXPECT_EQ(subm[i], 0);
    }
  }
}

TEST(ConcatNlsbBits, Test1) {
  vector<uint64_t> in = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
  auto out = concat_N_lsb_bits<1>(in);
  vector<byte> expected = {0b10101010};
  for (size_t i = 0; i < out.size(); i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(ConcatNlsbBits, Test2) {
  vector<uint64_t> in = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
  auto out = concat_N_lsb_bits<2>(in);
  vector<byte> expected = {0b01101100, 0b01101100};
  for (size_t i = 0; i < out.size(); i++) {
    EXPECT_EQ(out[i], expected[i]);
  }
}

TEST(ConcatNlsbBits, Test3) {
  vector<uint64_t> in = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
  auto out = concat_N_lsb_bits<3>(in);
  vector<byte> expected = {0b00101001, 0b11001011, 0b10111000};
  for (size_t i = 0; i < out.size(); i++) {
    EXPECT_EQ(static_cast<bitset<8>>(out[i]),
              static_cast<bitset<8>>(expected[i]));
  }
}