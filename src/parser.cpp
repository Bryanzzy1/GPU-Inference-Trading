#include "parser.hpp"

#include <charconv>   // std::from_chars
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {

// Parse one CSV line into `out`. Returns true on success.
// Expected columns (no header):
//   agg_trade_id, price, qty, first_trade_id, last_trade_id,
//   timestamp(us), is_buyer_maker, is_best_match
// We read the ones we keep and skip the rest.
bool parse_line(const char* begin, const char* end, Trade& out) {
    const char* p = begin;

    // Helper: find the next comma at or after p (bounded by end).
    auto next_comma = [&](const char* from) -> const char* {
        const char* c = from;
        while (c < end && *c != ',') ++c;
        return c;
    };

    // Field 0: agg_trade_id (int64)
    const char* c = next_comma(p);
    if (std::from_chars(p, c, out.agg_id).ec != std::errc{}) return false;
    if (c >= end) return false;
    p = c + 1;

    // Field 1: price (double)
    c = next_comma(p);
    if (std::from_chars(p, c, out.price).ec != std::errc{}) return false;
    if (c >= end) return false;
    p = c + 1;

    // Field 2: qty (double)
    c = next_comma(p);
    if (std::from_chars(p, c, out.qty).ec != std::errc{}) return false;
    if (c >= end) return false;
    p = c + 1;

    // Field 3: first_trade_id — skip
    c = next_comma(p);
    if (c >= end) return false;
    p = c + 1;

    // Field 4: last_trade_id — skip
    c = next_comma(p);
    if (c >= end) return false;
    p = c + 1;

    // Field 5: timestamp (int64, microseconds)
    c = next_comma(p);
    if (std::from_chars(p, c, out.time_us).ec != std::errc{}) return false;
    if (c >= end) return false;
    p = c + 1;

    // Field 6: is_buyer_maker — literal text "True"/"False"
    c = next_comma(p);
    {
        const std::size_t len = static_cast<std::size_t>(c - p);
        // Compare against "True" (4 chars). Anything else is treated as false,
        // but we require it to actually be "False" to consider the line valid.
        if (len == 4 && p[0] == 'T' && p[1] == 'r' && p[2] == 'u' && p[3] == 'e') {
            out.is_buyer_maker = true;
        } else if (len == 5 && p[0] == 'F' && p[1] == 'a' && p[2] == 'l' &&
                   p[3] == 's' && p[4] == 'e') {
            out.is_buyer_maker = false;
        } else {
            return false;  // unexpected token in the boolean field
        }
    }
    // Field 7 (is_best_match) is ignored; we don't need to read past field 6.

    return true;
}

}  // namespace

std::vector<Trade> parse_agg_trades(const std::string& path, std::size_t& bad_lines) {
    bad_lines = 0;

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        throw std::runtime_error("cannot open file: " + path);
    }

    // Read the whole file into one buffer. One big read beats many small ones.
    std::ostringstream ss;
    ss << f.rdbuf();
    const std::string buf = ss.str();

    std::vector<Trade> trades;
    trades.reserve(500000);  // ~473k rows in a day of BTCUSDT; allocate once

    const char* p = buf.data();
    const char* end = p + buf.size();

    while (p < end) {
        // Find end of this line.
        const char* nl = p;
        while (nl < end && *nl != '\n') ++nl;

        // Trim a trailing '\r' (Windows CRLF) so the last field parses cleanly.
        const char* line_end = nl;
        if (line_end > p && *(line_end - 1) == '\r') --line_end;

        if (line_end > p) {  // skip blank lines
            Trade t;
            if (parse_line(p, line_end, t)) {
                trades.push_back(t);
            } else {
                ++bad_lines;
            }
        }

        p = (nl < end) ? nl + 1 : end;
    }

    return trades;
}

std::vector<Trade> parse_agg_trades(const std::string& path) {
    std::size_t ignored = 0;
    return parse_agg_trades(path, ignored);
}
