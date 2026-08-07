#include <sstream>
#include <filesystem>
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <QDebug>
jmp_buf env;
using namespace Infrastructure::PDF;

namespace {
constexpr float kTableRowHeight = 18.0f;
constexpr float kDescriptionColumnWidth = 250.0f;
constexpr float kFirstPageTableY = 650.0f;
constexpr float kContinuationTableTopOffset = 145.0f;
constexpr float kTableHeaderGap = 10.0f;
constexpr float kFooterLimitY = 72.0f;
constexpr float kFooterLineY = 54.0f;
constexpr int kTitleFontSize = 32;
constexpr int kContinuationTitleFontSize = 24;
constexpr int kBodyFontSize = 12;
constexpr int kFooterFontSize = 8;
} // namespace

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

void InvoicePdfGenerator::save_pdf(HPDF_Doc pdf, const char* name) {
    HPDF_SaveToFile(pdf, name);
}

std::string InvoicePdfGenerator::retrieveFileName() {
    std::string name;
    std::cout << "\nPlease enter the name of the file: ";
    std::cin >> name;
    return name;
}

void InvoicePdfGenerator::draw_invoice_table_header(HPDF_Page page,
                                                    const TableHeader& table_header,
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
        float total = item.getStdPrice() * stock_amount;

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << total;
        drawCell(layout.colWidths[3], ss.str());
    }
}

InvoicePdfGenerator::TableHeader InvoicePdfGenerator::build_invoice_table_header(float page_width) const {
    TableHeader headers;
    const float usable_width = page_width - (peece_margin * 2.0f) - kDescriptionColumnWidth;
    headers.x = peece_margin;
    headers.y = 0.0f;
    headers.rowHeight = kTableRowHeight;
    headers.colWidths[0] = kDescriptionColumnWidth;
    headers.colWidths[1] = usable_width / 3.0f;
    headers.colWidths[2] = usable_width / 3.0f;
    headers.colWidths[3] = usable_width / 3.0f;
    headers.strings[0] = "Description";
    headers.strings[1] = "Quantity Supplied";
    headers.strings[2] = "Item Price";
    headers.strings[3] = "Total Price";
    return headers;
}

InvoicePdfGenerator::TableLayout InvoicePdfGenerator::build_invoice_table_layout(float page_width,
                                                                                 float page_height,
                                                                                 bool first_page) const {
    TableLayout table;
    const float usable_width = page_width - (peece_margin * 2.0f) - kDescriptionColumnWidth;
    table.x = peece_margin;
    table.y = first_page ? kFirstPageTableY : page_height - kContinuationTableTopOffset;
    table.rowHeight = kTableRowHeight;
    table.colWidths[0] = kDescriptionColumnWidth;
    table.colWidths[1] = usable_width / 3.0f;
    table.colWidths[2] = usable_width / 3.0f;
    table.colWidths[3] = usable_width / 3.0f;
    return table;
}

HPDF_Page InvoicePdfGenerator::start_invoice_table_page(HPDF_Doc pdf,
                                                        bool first_page,
                                                        int page_number,
                                                        TableLayout& table) {
    HPDF_Page page = first_page ? HPDF_GetPageByIndex(pdf, 0) : addPage(pdf);
    const float page_width = HPDF_Page_GetWidth(page);
    const float page_height = HPDF_Page_GetHeight(page);
    TableHeader headers = build_invoice_table_header(page_width);

    draw_invoice_page_header(pdf, page, first_page);
    draw_invoice_footer(pdf, page, page_number);

    table = build_invoice_table_layout(page_width, page_height, first_page);

    HPDF_Page_SetFontAndSize(page, HPDF_GetFont(pdf, "Helvetica-Bold", NULL), kBodyFontSize);
    draw_invoice_table_header(page, headers, headers.x, table.y + table.rowHeight + kTableHeaderGap);
    HPDF_Page_SetFontAndSize(page, HPDF_GetFont(pdf, "Helvetica", NULL), kBodyFontSize);
    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);

    return page;
}

void InvoicePdfGenerator::draw_invoice_page_header(HPDF_Doc pdf, HPDF_Page page, bool first_page) {
    const float page_width = HPDF_Page_GetWidth(page);
    const float page_height = HPDF_Page_GetHeight(page);
    const float title_y = first_page ? page_height - 62.0f : page_height - 42.0f;
    const float details_x = first_page ? 100.0f : peece_margin;
    const float details_y = first_page ? page_height - 44.0f : page_height - 42.0f;
    const float link_y = details_y - 18.0f;
    const char* tax_inv = "Tax Invoice";

    HPDF_Font regular = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Font bold = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetTextRenderingMode(page, HPDF_FILL);
    HPDF_Page_SetFontAndSize(page, regular, first_page ? kTitleFontSize : kContinuationTitleFontSize);
    const float title_width = HPDF_Page_TextWidth(page, tax_inv);
    HPDF_Page_TextOut(page, page_width - peece_margin - title_width, title_y, tax_inv);
    HPDF_Page_EndText(page);

    std::string business_name;
    std::string website;
    std::string logo_path;
    if (cur_invoice_ && cur_invoice_->getBusiness()) {
        const auto& business = cur_invoice_->getBusiness();
        business_name = business->getName().empty() ? business->getBizName() : business->getName();
        website = business->get_website_url();
        logo_path = business->get_biz_logo_url();
    }
    if (website.empty() && cur_invoice_) {
        website = cur_invoice_->get_website();
    }

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, bold, kBodyFontSize);
    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    if (!business_name.empty()) {
        HPDF_Page_TextOut(page, details_x, details_y, business_name.c_str());
    }
    if (!website.empty()) {
        HPDF_Page_SetFontAndSize(page, regular, kBodyFontSize);
        HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 1.0f);
        HPDF_Page_TextOut(page, details_x, link_y, website.c_str());
    }
    HPDF_Page_EndText(page);

    if (!website.empty()) {
        underline_word(page, website.c_str(), 0.1f, details_x, link_y, 0.0f, 0.0f, 1.0f);
    }

    if (first_page && !logo_path.empty() && std::filesystem::exists(logo_path)) {
        HPDF_Image logo = HPDF_LoadPngImageFromFile(pdf, logo_path.c_str());
        const float img_width_max = 50.0f;
        const float img_height_max = 100.0f;
        const float x_placement = 50.0f;
        const float y_placement = page_height - 72.0f;
        resize_and_place_image(page, img_width_max, img_height_max, x_placement, y_placement, logo);
    }

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);
}

void InvoicePdfGenerator::draw_invoice_footer(HPDF_Doc pdf, HPDF_Page page, int page_number) {
    const float page_width = HPDF_Page_GetWidth(page);
    const std::string page_text = "Page " + std::to_string(page_number);

    HPDF_Page_SetLineWidth(page, 0.25f);
    HPDF_Page_SetRGBStroke(page, 0.72f, 0.72f, 0.72f);
    HPDF_Page_MoveTo(page, peece_margin, kFooterLineY);
    HPDF_Page_LineTo(page, page_width - peece_margin, kFooterLineY);
    HPDF_Page_Stroke(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, HPDF_GetFont(pdf, "Helvetica", NULL), kFooterFontSize);
    HPDF_Page_SetRGBFill(page, 0.35f, 0.35f, 0.35f);
    const float page_text_width = HPDF_Page_TextWidth(page, page_text.c_str());
    HPDF_Page_TextOut(page, page_width - peece_margin - page_text_width, kFooterLineY - 16.0f, page_text.c_str());
    HPDF_Page_EndText(page);

    HPDF_Page_SetRGBFill(page, 0.0f, 0.0f, 0.0f);
    HPDF_Page_SetRGBStroke(page, 0.0f, 0.0f, 0.0f);
}

bool InvoicePdfGenerator::row_fits_on_page(const TableLayout& table, int rowIndex) const {
    const float row_bottom = table.y - ((rowIndex + 1) * table.rowHeight);
    return row_bottom >= kFooterLimitY;
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
    HPDF_Doc peeceInvoicePDF = nullptr;
    try {
        if (!cur_invoice_ || cur_invoice_->get_file_name().empty()) {
            return false;
        }

        peeceInvoicePDF = createPDF("Helvetica");
        if (!peeceInvoicePDF) {
            return false;
        }

        TableLayout table;
        int page_number = 1;
        int row_index = 0;
        auto page = start_invoice_table_page(peeceInvoicePDF, true, page_number, table);

        for (const auto& stock : cur_invoice_->getStockQuantityMap()) {
            if (!stock.first) {
                continue;
            }

            if (!row_fits_on_page(table, row_index)) {
                page = start_invoice_table_page(peeceInvoicePDF, false, ++page_number, table);
                row_index = 0;
            }

            std::cout << stock.first->getName() << " " << stock.second << " x " << stock.first->getStdPrice()
                      << "\n";
            draw_stock_item_row(page, *stock.first, stock.second, table, row_index);
            row_index++;
        }

        save_pdf(peeceInvoicePDF, cur_invoice_->get_file_name().c_str());
        HPDF_Free(peeceInvoicePDF);
        peeceInvoicePDF = nullptr;
        qDebug() << "Saved PDF to" << cur_invoice_->get_file_name().c_str();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Error at peece template: " << e.what();
    } catch (...) {
        std::cerr << "Error in the peece template\n";
    }

    if (peeceInvoicePDF) {
        HPDF_Free(peeceInvoicePDF);
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
