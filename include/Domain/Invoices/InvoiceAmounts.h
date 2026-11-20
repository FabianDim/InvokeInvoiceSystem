#pragma once
#include <cmath>

namespace Domain::Invoices {
// Round each extended line and its GST to cents before summing the invoice.
inline double money(double value) { return std::round(value * 100.0) / 100.0; }
struct Amounts {
    double subtotal = 0;
    double gst = 0;
    double total() const { return money(subtotal + gst); }
    void add(double price, int quantity, bool registered) {
        const auto net = money(price * quantity);
        subtotal = money(subtotal + net);
        const auto cents = std::llround(net * 100.0);
        // Integer rounding avoids binary floating-point ties such as $1.15 GST.
        const auto tax = registered ? static_cast<double>((cents + 5) / 10) / 100.0 : 0.0;
        gst = money(gst + tax);
    }
};
}
