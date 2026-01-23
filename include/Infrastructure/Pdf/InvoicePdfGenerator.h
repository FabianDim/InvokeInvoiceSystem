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

  public:
    HPDF_Doc createPDF(const char* font);
    HPDF_Page addPage(HPDF_Doc pdf);
    void addPageBefore(HPDF_Doc pdf, HPDF_Page page_1);
    void savePDF(HPDF_Doc pdf, const char* name);
    void createTestPDF(HPDF_Doc pdf, HPDF_Page page_1);
    std::string retrieveFileName();
    void draw_stock_item_row(HPDF_Page page, const StockItem& item, const TableLayout& layout, int rowIndex);
    bool peece_template();

    InvoicePdfGenerator(std::shared_ptr<Invoice> invoice);

  private:
    // std::shared_ptr<Invoice> curInvoice;
    static const std::set<std::string> fontList;
    HPDF_Font def_font;
    void resize_place_image(HPDF_Page page,
                            const float img_width_max,
                            const float img_height_max,
                            const float place_x,
                            const float place_y,
                            HPDF_Image logo);

    std::shared_ptr<Invoice> cur_invoice_;
};

} // namespace Infrastructure::PDF