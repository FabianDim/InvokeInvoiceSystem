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

std::string InvoicePdfGenerator::retrieveFileName() {
    std::string name;
    std::cout << "\nPlease enter the name of the file: ";
    std::cin >> name;
    return name;
}

void InvoicePdfGenerator::draw_invoice_table_header(HPDF_Page page,
                                                    TableHeader table_header,
                                                    float start_x,
                                                    const float start_y) {

    auto drawCell = [&](float width, const std::string& text) {
        // Cell border
        HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);
        HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
        HPDF_Page_Rectangle(page, start_x, start_y, width, table_header.rowHeight);
        HPDF_Page_Stroke(page);
        // HPDF_Page_Fill(page);
        // Text
        HPDF_Page_BeginText(page);

        HPDF_Page_TextRect(page,
                           start_x + 4.0f, // left padding
                           start_y + 16,   // top padding
                           start_x + width,
                           start_y + 4,
                           text.c_str(),
                           HPDF_TALIGN_LEFT,
                           nullptr);

        HPDF_Page_EndText(page);

        start_x += width;
    };
    int i = 0;
    for (const auto& head : table_header.strings) {
        drawCell(table_header.colWidths[i], head);
        i++;
    }
}

void InvoicePdfGenerator::draw_stock_item_row(
    HPDF_Page page, const StockItem& item, const int stock_amount, const TableLayout& layout, int rowIndex) {
    const float yTop = layout.y - rowIndex * layout.rowHeight;
    const float yBottom = yTop - layout.rowHeight;

    float x = layout.x;

    auto drawCell = [&](float width, const std::string& text) {
        // Cell border
        HPDF_Page_SetRGBStroke(page, 1.0f, 1.0f, 1.0f);
        HPDF_Page_Rectangle(page, x, yBottom, width, layout.rowHeight);
        HPDF_Page_Stroke(page);

        // Text
        HPDF_Page_BeginText(page);
        HPDF_Page_TextRect(page,
                           x + 4.0f,    // left padding
                           yTop + 4.0f, // top padding
                           x + width - 4.0f,
                           yBottom,
                           text.c_str(),
                           HPDF_TALIGN_LEFT,
                           nullptr);
        HPDF_Page_EndText(page);

        x += width;
    };
    drawCell(layout.colWidths[0], item.getName());
    drawCell(layout.colWidths[1], std::to_string(stock_amount));
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << item.getStdPrice();
        drawCell(layout.colWidths[2], ss.str());
    }
    {
        const auto& stock_map = cur_invoice_->getStockQuantityMap();
        float total = item.getStdPrice() * stock_amount;

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
    const int biz_details_spacing = 18;
    const int section_spacing = 32;
    try {
        auto peeceInvoicePDF = createPDF("Helvetica");
        auto page = HPDF_GetPageByIndex(peeceInvoicePDF, 0);
        HPDF_Font font = HPDF_Page_GetCurrentFont(page);

        // begin the Tax Invoice text output.
        HPDF_Page_BeginText(page);
        HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);

        HPDF_Page_SetFontAndSize(page, font, h2);
        // HPDF_Page_SetTextLeading(page, 18);
        const char* tax_inv = "Tax Invoice";
        float width = HPDF_Page_TextWidth(page, tax_inv);
        float page_right = HPDF_Page_GetWidth(page);
        // place the tax invoice 50 from the end of the page
        float x_tax_inv_place = page_right - peece_margin - width;

        HPDF_Page_TextOut(page, x_tax_inv_place, 780, tax_inv);
        HPDF_Page_EndText(page);

        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, font, body);
        HPDF_Page_TextOut(page, 100, 830 - h2, "");

        HPDF_Page_SetTextLeading(page, biz_details_spacing);
        try {
            const auto& bizName = cur_invoice_->getBusiness()->getBizName();
            const auto& name = cur_invoice_->getBusiness()->getName();
            const auto& biz_logo = cur_invoice_->getBusiness()->get_biz_logo_url();
            HPDF_Page_SetFontAndSize(page, HPDF_GetFont(peeceInvoicePDF, "Helvetica-Bold", NULL), body);
            if (name == "") {
                HPDF_Page_ShowText(page, cur_invoice_->getBusiness()->getBizName().c_str());
            } else {
                HPDF_Page_ShowText(page, cur_invoice_->getBusiness()->getName().c_str());
            }
            HPDF_Page_MoveToNextLine(page);
            // show link blue
            const char* link = cur_invoice_->getBusiness()->get_website_url().c_str();
            HPDF_Page_SetFontAndSize(page, HPDF_GetFont(peeceInvoicePDF, "Helvetica", NULL), body);
            HPDF_Page_SetRGBFill(page, 0, 0, 1);
            const auto& cur_pos = HPDF_Page_GetCurrentTextPos(page);
            HPDF_Page_ShowText(page, link);
            HPDF_Image logo = HPDF_LoadPngImageFromFile(peeceInvoicePDF, biz_logo.c_str());
            HPDF_Page_EndText(page);
            underline_word(page, link, 0.1, cur_pos.x, cur_pos.y, 0.0f, 0.0f, 1.0f);

            const float img_width_max = 50.0f;
            const float img_height_max = 100.0f;
            if (logo) {
                float x_placement = 50.0f;
                float y_placement = 770.0f;
                resize_and_place_image(page, img_width_max, img_height_max, x_placement, y_placement, logo);
            }

            TableLayout table;
            table.x = peece_margin;
            table.y = 650.0f; // start near top
            table.rowHeight = 18.0f;
            table.colWidths[0] = 250.0f;
            const float useable_width = (page_right - peece_margin) - (0 + peece_margin + table.colWidths[0]);
            table.colWidths[1] = useable_width / 3;
            table.colWidths[2] = useable_width / 3;
            table.colWidths[3] = useable_width / 3;
            TableHeader headers;
            headers.colWidths[0] = 250.0f;
            headers.colWidths[1] = useable_width / 3;
            headers.colWidths[2] = useable_width / 3;
            headers.colWidths[3] = useable_width / 3;
            headers.rowHeight = 18.0f;
            headers.strings[0] = "Description";
            headers.strings[1] = "Quantity Supplied";
            headers.strings[2] = "Item Price";
            headers.strings[3] = "Total Price";
            draw_invoice_table_header(page, headers, peece_margin, 660 + headers.rowHeight);
            HPDF_Page_SetRGBFill(page, 0, 0, 0);
            HPDF_Page_SetRGBStroke(page, 0, 0, 0);
            int i = 0;
            HPDF_Page_SetFontAndSize(page, HPDF_GetFont(peeceInvoicePDF, "Helvetica", NULL), body);
            for (const auto& stock : cur_invoice_->getStockQuantityMap()) {
                std::cout << stock.first->getName() << " " << stock.second << " x " << stock.first->getStdPrice()
                          << "\n";
                draw_stock_item_row(page, *stock.first, stock.second, table, i);
                i++;
            }

            savePDF(peeceInvoicePDF, cur_invoice_->get_file_name().c_str());
            qDebug() << "Saved PDF to" << cur_invoice_->get_file_name().c_str();
        } catch (std::exception e) {
            qDebug() << "Exception at peece template: " << e.what();
        }
    } catch (...) {
        std::cerr << "Error in the peece template\n";
    }
    return false;
}

void InvoicePdfGenerator::underline_word(HPDF_Page page,
                                         const char* text,
                                         const float line_width,
                                         const float x,
                                         const float y,
                                         const float r,
                                         const float g,
                                         const float b) {

    /**
     * @brief helper function to underline a string of text.
     *
     * @param text Text to be underlined.
     * @param line_width Width of the underline.
     * @param x x-axis of the line
     * @param y y-axis of the line
     * @param rgb the colour of the line from 0-1
     *
     * @return Bool flag of successful completion
     * @pre An instance of this class should be created.
     */
    float text_width = HPDF_Page_TextWidth(page, text);
    float underlineY = y - 3.0f;

    // Draw underline
    HPDF_Page_SetRGBStroke(page, r, g, b);
    HPDF_Page_SetLineWidth(page, line_width);
    HPDF_Page_MoveTo(page, x, underlineY);
    HPDF_Page_LineTo(page, x + text_width, underlineY);
    HPDF_Page_Stroke(page);
}

void InvoicePdfGenerator::resize_and_place_image(HPDF_Page page,
                                                 const float img_width_max,
                                                 const float img_height_max,
                                                 const float place_x,
                                                 const float place_y,
                                                 HPDF_Image logo) {
    /**
     * @brief Resizes an image and places the image on the page.
     *
     *
     * @param page
     * @param img_width_max
     * @param img_height_max
     * @param logo
     *
     * @pre An instance of this class should be created.
     */
    HPDF_REAL imgW = HPDF_Image_GetWidth(logo);
    HPDF_REAL imgH = HPDF_Image_GetHeight(logo);
    float aspect_ratio = imgW / imgH;
    float image_height = imgH;
    float image_width = imgW;

    float scale_w = img_width_max / imgW;
    float scale_h = img_height_max / imgH;

    float scale = std::min(scale_w, scale_h);

    image_width = imgW * scale;
    image_height = imgH * scale;

    float x_placement = place_x - (image_width / 2);

    HPDF_Page_DrawImage(page, logo, x_placement, place_y, image_width, image_height);
}
// http://libharu.org/demo/text_demo.c
// http://libharu.org/demo/line_demo.c
// https://github.com/libharu/libharu/wiki/Examples#user-content-font_democ
// https://github.com/libharu/libharu/wiki/Error-handling errors
// https://johan162.github.io/libhpdftbl/html/index.html
// https://libharu.org/demo/text_demo.pdf - Grid
// https://math.stackexchange.com/questions/3078121/resizing-and-scaling-image resizing image