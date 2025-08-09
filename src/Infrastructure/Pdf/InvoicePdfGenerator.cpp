#include <sstream>
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"

jmp_buf env;

const std::set<std::string> InvoicePdfGenerator::fontList{
    "Courier",
    "Courier-Bold",
    "Courier-Oblique",
    "Courier-BoldOblique",
    "Helvetica",
    "Helvetica-Bold",
    "Helvetica-Oblique",
    "Helvetica-BoldOblique",
    "Times-Roman",
    "Times-Bold",
    "Times-Italic",
    "Times-BoldItalic",
    "Symbol",
    "ZapfDingbats"
};

void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* user_data) {
    std::cerr << "Error: " << std::hex << error_no << ", Detail: " << detail_no << std::endl;
}

HPDF_Doc InvoicePdfGenerator::createPDF(const char* font) {
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);
    if (!pdf) {
        printf("ERROR: cannot create pdf object.\n");
        return nullptr;
    }
    if (setjmp(env)) {
        HPDF_Free(pdf);
        return nullptr;
    }
    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    if (fontList.contains(font)) {
        def_font = HPDF_GetFont(pdf, font, NULL);
    }
    else {
        std::cerr << "Cannot find that font" << std::endl;
        return NULL;
    }

    /* set page mode to use outlines. */
    HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_USE_OUTLINE);

    HPDF_Page page = addPage(pdf);
    HPDF_Page_SetFontAndSize(page, def_font, 24);
    return pdf;
}

HPDF_Page InvoicePdfGenerator::addPage(HPDF_Doc pdf) {
    return HPDF_AddPage(pdf);
}

void InvoicePdfGenerator::addPageBefore(HPDF_Doc pdf, HPDF_Page page_1) {
    HPDF_Page page_0;

    page_0 = HPDF_InsertPage(pdf, page_1);
}

void InvoicePdfGenerator::savePDF(HPDF_Doc pdf, const char* name) {
    HPDF_SaveToFile(pdf, name);
}

void InvoicePdfGenerator::createTestPDF(HPDF_Doc pdf, HPDF_Page page_1) {  
    HPDF_Page_BeginText(page_1);  
    HPDF_Page_TextOut(page_1, 60, 140, "test"); // page, xpos, ypos, text  // Corrected invocation  
    HPDF_Page_EndText(page_1);  
    savePDF(pdf, "test.pdf");  
}

std::string InvoicePdfGenerator::retrieveFileName() {
    std::string name;
    std::cout << "\nPlease enter the name of the file: ";
    std::cin >> name;
    return name;
}

bool InvoicePdfGenerator::peeceTemplate(std::shared_ptr<Invoice> curInvoice) {
    try {
       auto peeceInvoicePDF = createPDF("Helvetica");
       auto page = addPage(peeceInvoicePDF);
       HPDF_Font font = HPDF_Page_GetCurrentFont(page);
       HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);
       int h2 = 32;
       HPDF_Page_SetFontAndSize(page, font, h2);
       const char* invoiceType = "Invoice";
       HPDF_Page_TextOut(page, 450, 820, invoiceType);
       HPDF_Page_TextOut(page, 450, 750, curInvoice->getBusiness()->getBizName().c_str());
       HPDF_Page_EndText(page);
       savePDF(peeceInvoicePDF, retrieveFileName().c_str());


    }
    catch (...) { 
        std::cout << "Error in the peece template\n"; 
    }
    return false;
}


//http://libharu.org/demo/text_demo.c
//http://libharu.org/demo/line_demo.c
//https://github.com/libharu/libharu/wiki/Examples#user-content-font_democ
// https://github.com/libharu/libharu/wiki/Error-handling errors
// https://johan162.github.io/libhpdftbl/html/index.html