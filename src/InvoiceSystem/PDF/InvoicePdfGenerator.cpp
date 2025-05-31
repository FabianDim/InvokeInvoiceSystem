#include <sstream>
jmp_buf env;

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

    addPage(pdf);
    HPDF_Page_SetFontAndSize(HPDF_GetCurrentPage(pdf), def_font, 24);
    return pdf;
}

void InvoicePdfGenerator::addPage(HPDF_Doc pdf) {
    HPDF_Page page_1;

    page_1 = HPDF_AddPage(pdf);
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
    HPDF_Page_TextOut(page_1, 60, 140, "test"); //page, xpos, ypos, text 
    HPDF_Page_EndText(page_1);
    savePDF(pdf, "test.pdf");
}



//http://libharu.org/demo/text_demo.c
//http://libharu.org/demo/line_demo.c
//https://github.com/libharu/libharu/wiki/Examples#user-content-font_democ
// https://github.com/libharu/libharu/wiki/Error-handling errors