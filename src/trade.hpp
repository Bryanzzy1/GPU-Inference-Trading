#pragma once
#include <cstdint>

// One aggregated trade from Binance spot aggTrades.
// POD (plain old data): trivially copyable, predictable layout — the hot path
// later treats these as raw bytes.
//
// The source CSV has 8 columns; we keep only what M1 needs. Dropped:
// first_trade_id / last_trade_id / is_best_match (no signal for our features).
struct Trade {
    int64_t agg_id;          // aggTrade id: identity / ordering, not a feature
    double  price;
    double  qty;
    int64_t time_us;         // exchange timestamp, MICROSECONDS since epoch
    bool    is_buyer_maker;  // true => seller was the aggressor (sell pressure)
};
