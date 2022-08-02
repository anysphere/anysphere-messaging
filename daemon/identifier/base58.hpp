//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

// From https://github.com/bitcoin/bitcoin/blob/master/src/base58.h
// Modified to fit into our infrastructure.
// See bitcoin.LICENSE for license.

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Why base-58 instead of standard base-64 encoding?
 * - Don't want 0OIl characters that look the same in some fonts and
 *      could be used to create visually identical looking data.
 * - A string with non-alphanumeric characters is not as easily accepted as
 * input.
 * - E-mail usually won't line-break if there's no punctuation to break at.
 * - Double-clicking selects the whole string as one word if it's all
 * alphanumeric.
 */

#include "asphr/asphr.hpp"

namespace base58 {

/**
 * Encode any byte sequence (not just utf8!) as a base58-encoded string.
 */
auto Encode(string_view input) -> string;

/**
 * Decode a base58-encoded string into a byte string.
 */
auto Decode(string_view str) -> asphr::StatusOr<string>;

}  // namespace base58