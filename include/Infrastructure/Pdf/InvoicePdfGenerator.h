#pragma once
#include "pch.h"
#include "Application/Accounts/AccountManager.h"
#include <hpdf.h>
#include <setjmp.h>
#include "Domain/Invoices/Invoice.h"

namespace Infrastructure::PDF {

class InvoicePdfGenerator {
    friend class InvoiceMenu;

  public:
    HPDF_Doc createPDF(const char* font);
    HPDF_Page addPage(HPDF_Doc pdf);
    void addPageBefore(HPDF_Doc pdf, HPDF_Page page_1);
    void savePDF(HPDF_Doc pdf, const char* name);
    void createTestPDF(HPDF_Doc pdf, HPDF_Page page_1);
    std::string retrieveFileName();
    bool peeceTemplate();
    InvoicePdfGenerator(std::shared_ptr<Invoice> invoice);

  private:
    // std::shared_ptr<Invoice> curInvoice;
    static const std::set<std::string> fontList;
    HPDF_Font def_font;

    std::shared_ptr<Invoice> cur_invoice_;
};

} // namespace Infrastructure::PDF