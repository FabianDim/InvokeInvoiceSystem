#pragma once
#include "pch.h"
#include "Application/Accounts/AccountManager.h"
#include <hpdf.h>
#include <setjmp.h>
#include "Domain/Invoices/Invoice.h"

namespace Infrastructure::PDF {

class InvoicePdfGenerator {
    friend class InvoiceMenu;
    struct TableLayout {
        float x;
        float y;
        float rowHeight;
        float colWidths[4];
    };
    struct TableHeader {
        float x;
        float y;
        float rowHeight;
        float colWidths[4];
        std::string strings[4];
    };

  public:
    HPDF_Doc createPDF(const char* font);
    HPDF_Page addPage(HPDF_Doc pdf);
    void addPageBefore(HPDF_Doc pdf, HPDF_Page page_1);
    void savePDF(HPDF_Doc pdf, const char* name);
    void createTestPDF(HPDF_Doc pdf, HPDF_Page page_1);
    std::string retrieveFileName();
    void draw_invoice_table_header(HPDF_Page page, TableHeader table_header, float start_x, const float start_y);
    void draw_stock_item_row(
        HPDF_Page page, const StockItem& item, const int stock_amount, const TableLayout& layout, int rowIndex);
    bool peece_template();

    void underline_word(HPDF_Page page,
                        const char* text,
                        const float line_width,
                        const float x,
                        const float y,
                        const float r,
                        const float g,
                        const float b);

    InvoicePdfGenerator(std::shared_ptr<Invoice> invoice);

  private:
    // std::shared_ptr<Invoice> curInvoice;
    static const std::set<std::string> fontList;
    HPDF_Font def_font;
    void resize_and_place_image(HPDF_Page page,
                                const float img_width_max,
                                const float img_height_max,
                                const float place_x,
                                const float place_y,
                                HPDF_Image logo);
    const float peece_margin = 25.0f;
    std::shared_ptr<Invoice> cur_invoice_;
};

} // namespace Infrastructure::PDF