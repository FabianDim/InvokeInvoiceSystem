#pragma once
#include "pch.h"
#include "Accounts/AccountManager.h"
#include <hpdf.h>
#include <setjmp.h>
class InvoicePdfGenerator{
	friend class InvoiceMenu;
public:
    HPDF_Doc createPDF(const char* font);
	HPDF_Page addPage(HPDF_Doc pdf);
	void addPageBefore(HPDF_Doc pdf, HPDF_Page page_1);
	void savePDF(HPDF_Doc pdf, const char* name);
	void createTestPDF(HPDF_Doc pdf, HPDF_Page page_1);
private:
	static const std::set<std::string> fontList;

    HPDF_Font def_font;
};

