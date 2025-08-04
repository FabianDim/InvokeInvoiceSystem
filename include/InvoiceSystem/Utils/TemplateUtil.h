#pragma once

#include "InvoiceSystem/InvoiceData/InvoiceTemplateEnum.h"

inline std::string to_string(InvoiceTemplateEnum tmpl) {
    switch (tmpl) {
    case InvoiceTemplateEnum::PEECE: return "PEECE";
    default: return "UNKNOWN";
    }
}

inline InvoiceTemplateEnum to_enum(const std::string tmpl) {
    if (tmpl == "peece") {
        return InvoiceTemplateEnum::PEECE;
    }
    else {
        std::cerr << "Not a valid template\n";
        return InvoiceTemplateEnum::PEECE; //default;
    }
}

const std::unordered_map<int, std::string> invoiceTemplate{
    {1, "Peece"}
};