#include <sstream>
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <QDebug>
jmp_buf env;
using namespace Infrastructure::PDF;
Infrastructure::PDF::InvoicePdfGenerator::InvoicePdfGenerator(std::shared_ptr<Invoice> invoice)
    : cur_invoice_(invoice) {}

const std::set<std::string> InvoicePdfGenerator::fontList{"Courier",
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
                                                          "ZapfDingbats"};

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
        if (!def_font) {
            std::cerr << "HPDF_GetFont failed for font: " << font << std::endl;
            HPDF_Free(pdf);
            return nullptr;
        }
    } else {
        std::cerr << "Cannot find that font: " << font << std::endl;
        HPDF_Free(pdf);
        return nullptr;
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
void InvoicePdfGenerator::draw_stock_item_row(HPDF_Page page,
                                              const StockItem& item,
                                              const TableLayout& layout,
                                              int rowIndex) {
    const float yTop = layout.y - rowIndex * layout.rowHeight;
    const float yBottom = yTop - layout.rowHeight;

    float x = layout.x;

    auto drawCell = [&](float width, const std::string& text) {
        // Cell border
        HPDF_Page_Rectangle(page, x, yBottom, width, layout.rowHeight);
        HPDF_Page_Stroke(page);

        // Text
        HPDF_Page_BeginText(page);
        HPDF_Page_TextRect(page,
                           x + 4.0f,    // left padding
                           yTop - 4.0f, // top padding
                           x + width - 4.0f,
                           yBottom + 4.0f,
                           text.c_str(),
                           HPDF_TALIGN_LEFT,
                           nullptr);
        HPDF_Page_EndText(page);

        x += width;
    };
    drawCell(layout.colWidths[0], item.getName());
    drawCell(layout.colWidths[1], std::to_string(item.get_invoice_stock()));
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << item.getStdPrice();
        drawCell(layout.colWidths[2], ss.str());
    }
    {
        float total = item.getStdPrice() * item.get_invoice_stock();

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << total;
        drawCell(layout.colWidths[3], ss.str());
    }
}
/**
 * @brief Build a invoice using a template named peece.
 *
 * Uses the libharu library to create and export a pdf invoice
 * using the data passed by the invoice service.
 *
 * @return Bool flag of successful completion
 * @pre An instance of this class should be created.
 */
bool InvoicePdfGenerator::peece_template() {
    const int h2 = 32;
    const int h3 = 24;
    const int body = 12;
    const int biz_details_spacing = 12;
    try {
        auto peeceInvoicePDF = createPDF("Helvetica");
        auto page = HPDF_GetPageByIndex(peeceInvoicePDF, 0);
        HPDF_Font font = HPDF_Page_GetCurrentFont(page);
        HPDF_Page_BeginText(page);
        HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);

        HPDF_Page_SetFontAndSize(page, font, h2);
        // HPDF_Page_SetTextLeading(page, 18);
        const char* tax_inv = "Tax Invoice";
        float width = HPDF_Page_TextWidth(page, tax_inv);
        float x = 550 - width;
        HPDF_Page_TextOut(page, x, 790, tax_inv);
        HPDF_Page_SetFontAndSize(page, font, body);
        HPDF_Page_TextOut(page, 100, 840 - h2, "");

        HPDF_Page_SetTextLeading(page, biz_details_spacing);
        HPDF_Page_ShowText(page, cur_invoice_->getBusiness()->getBizName().c_str());
        HPDF_Page_EndText(page);
        const TableLayout table;
        int i = 0;
        for (const auto& stock : cur_invoice_->getStockQuantityMap()) {
            draw_stock_item_row(page, *stock.first, table, i);
            i++;
        }

        savePDF(peeceInvoicePDF, cur_invoice_->get_file_name().c_str());
        qDebug() << "Saved PDF to" << cur_invoice_->get_file_name().c_str();

    } catch (...) {
        std::cout << "Error in the peece template\n";
    }
    return false;
}

// http://libharu.org/demo/text_demo.c
// http://libharu.org/demo/line_demo.c
// https://github.com/libharu/libharu/wiki/Examples#user-content-font_democ
// https://github.com/libharu/libharu/wiki/Error-handling errors
// https://johan162.github.io/libhpdftbl/html/index.html