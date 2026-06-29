#pragma once
#include <string>
#include <vector>
#include "trade.hpp"

// Parse a Binance aggTrades CSV (no header row) into a vector<Trade>.
// Throws std::runtime_error if the file cannot be opened.
// A malformed line is skipped and counted (see bad_lines below); parsing does
// not abort on one bad row.
std::vector<Trade> parse_agg_trades(const std::string& path, std::size_t& bad_lines);

// Convenience overload when you don't care about the bad-line count.
std::vector<Trade> parse_agg_trades(const std::string& path);
