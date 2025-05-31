#pragma once
#include "../../pch.h"
#include "Accounts/AccountManager.h"
#include <hpdf.h>
#include <setjmp.h>
class InvoicePdfGenerator{
	friend class InvoiceMenu;
public:
    HPDF_Doc createPDF(const char* font);
	void addPage(HPDF_Doc pdf);
	void addPageBefore(HPDF_Doc pdf, HPDF_Page page_1);
	void savePDF(HPDF_Doc pdf, const char* name);
	void createTestPDF(HPDF_Doc pdf, HPDF_Page page_1);
private:
    std::set<std::string> fontList{
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
    "ZapfDingbats"};

    HPDF_Font def_font;
};

