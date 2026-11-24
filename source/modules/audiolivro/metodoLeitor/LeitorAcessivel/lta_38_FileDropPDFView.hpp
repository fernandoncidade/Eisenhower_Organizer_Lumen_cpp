#pragma once

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_26_pdf_view.hpp"

namespace source::plugin::Audiolivro {

class FileDropPDFView : public PDFView
{
    Q_OBJECT

public:
    explicit FileDropPDFView(QWidget *parent = nullptr);

signals:
    void file_dropped(const QString &path);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

}  // namespace source::plugin::Audiolivro
