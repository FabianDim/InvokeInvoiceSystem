#include "InvoicePdfGenerator.h"

// Define the error handler function
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cerr << "Error: " << error_no << ", Detail: " << detail_no << std::endl;
}
