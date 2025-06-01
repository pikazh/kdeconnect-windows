#include "remotecommandslistitemdelegate.h"
#include "ui/textlayoututils.h"
#include "uicommon.h"

#include <QPainter>

RemoteCommandsListItemDelegate::RemoteCommandsListItemDelegate(QWidget *parent)
    : QStyledItemDelegate{parent}
{}

void RemoteCommandsListItemDelegate::paint(QPainter *painter,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    auto rect = opt.rect;

    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(rect, opt.palette.highlight());
        painter->setPen(opt.palette.highlightedText().color());
    } else if (opt.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, opt.palette.window());
    }

    const int leftMargin = 3;
    auto remaining_width = rect.width() - leftMargin;
    rect.setRect(rect.x() + leftMargin, rect.y(), remaining_width, rect.height());

    int title_height = 0;

    {
        painter->save();
        // Title painting
        auto title = index.data(RemoteCommandsListItemDataRoles::Name).toString();

        auto font = opt.font;
        font.setPointSize(font.pointSize() + 2);
        painter->setFont(font);
        title_height = QFontMetrics(font).height();

        // On the top, aligned to the left after the icon
        painter->drawText(rect.x(), rect.y() + title_height, title);

        painter->restore();
    }

    { // Description painting
        auto description = index.data(RemoteCommandsListItemDataRoles::Command).toString();

        QTextLayout text_layout(description, opt.font);

        qreal height = 0;
        auto cut_text = TextLayoutUtils::viewItemTextLayout(text_layout, remaining_width, height);

        // Get first line unconditionally
        description = cut_text.first().second;
        auto num_lines = 1;

        // Get second line, elided if needed
        if (cut_text.size() > 1) {
            // 2.5x so because there should be some margin left from the 2x so things don't get too squishy.
            if (rect.height() - title_height <= 2.5 * opt.fontMetrics.height()) {
                // If there's not enough space, show only a single line, elided.
                description = opt.fontMetrics.elidedText(description,
                                                         opt.textElideMode,
                                                         cut_text.at(0).first);
            } else {
                if (cut_text.size() > 2) {
                    description += opt.fontMetrics.elidedText(cut_text.at(1).second,
                                                              opt.textElideMode,
                                                              cut_text.at(1).first);
                } else {
                    description += cut_text.at(1).second;
                }
                num_lines += 1;
            }
        }

        int description_x = rect.x();

        // Have the y-value be set based on the number of lines in the description, to centralize the
        // description text with the space between the base and the title.
        int description_y = rect.y() + title_height + (rect.height() - title_height) / 2;
        if (num_lines == 1)
            description_y -= opt.fontMetrics.height() / 2;
        else
            description_y -= opt.fontMetrics.height();

        // On the bottom, aligned to the left after the icon, and featuring at most two lines of text (with some margin space to spare)
        painter->drawText(description_x,
                          description_y,
                          remaining_width,
                          cut_text.size() * opt.fontMetrics.height(),
                          Qt::TextWordWrap,
                          description);
    }

    painter->restore();
}
