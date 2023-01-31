#include "tree.h"

#include "tree_delegate.h"
#include "tree_header_view.h"
#include "tree_view.h"

#include <ui/design_system/design_system.h>
#include <ui/widgets/scroll_bar/scroll_bar.h>
#include <utils/helpers/color_helper.h>

#include <QVBoxLayout>


class Tree::Implementation
{
public:
    explicit Implementation(QWidget* _parent);


    QModelIndex lastClickedIndex;

    TreeView* tree = nullptr;
    QHeaderView* header = nullptr;
    TreeDelegate* delegate = nullptr;
    ScrollBar* treeScrollBar = nullptr;
};

Tree::Implementation::Implementation(QWidget* _parent)
    : tree(new TreeView(_parent))
    , header(new TreeHeaderView(_parent))
    , delegate(new TreeDelegate(_parent))
    , treeScrollBar(new ScrollBar(tree))
{
    tree->setHeader(header);
    tree->setHeaderHidden(true);
    tree->setAnimated(true);
    tree->setMouseTracking(true);
    tree->setFrameShape(QFrame::NoFrame);
    tree->setSelectionMode(QAbstractItemView::SingleSelection);
    tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    tree->setVerticalScrollBar(treeScrollBar);
    tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tree->setItemDelegate(delegate);

    header->setSectionResizeMode(QHeaderView::Stretch);
}


// ****


Tree::Tree(QWidget* _parent)
    : Widget(_parent)
    , d(new Implementation(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(d->tree);

    connect(d->tree, &TreeView::clicked, this, [this](const QModelIndex& _index) {
        const bool firstClick = d->lastClickedIndex != _index;
        d->lastClickedIndex = _index;
        emit clicked(_index, firstClick);
    });
    connect(d->tree, &TreeView::doubleClicked, this, &Tree::doubleClicked);
    connect(d->tree, &TreeView::expanded, this, &Tree::expanded);
    connect(d->tree, &TreeView::collapsed, this, &Tree::collapsed);
}

Tree::~Tree() = default;

void Tree::setModel(QAbstractItemModel* _model)
{
    if (model() != nullptr) {
        disconnect(d->tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
                   &Tree::currentIndexChanged);
    }

    d->tree->setModel(_model);

    if (model() != nullptr) {
        connect(d->tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
                &Tree::currentIndexChanged);
    }
}

QAbstractItemModel* Tree::model() const
{
    return d->tree->model();
}

int Tree::recursiveRowCount(bool _onlyExpanded) const
{
    int count = 0;
    std::function<void(const QModelIndex&)> recursiveCount;
    recursiveCount = [this, _onlyExpanded, &count, &recursiveCount](const QModelIndex& _parent) {
        for (int row = 0; row < model()->rowCount(_parent); ++row) {
            const auto index = model()->index(row, 0, _parent);
            ++count;

            if (!_onlyExpanded || isExpanded(index)) {
                recursiveCount(index);
            }
        }
    };
    recursiveCount({});
    return count;
}

void Tree::setRootIsDecorated(bool _decorated)
{
    d->tree->setRootIsDecorated(_decorated);
}

void Tree::setScrollBarVisible(bool _visible)
{
    d->tree->setVerticalScrollBarPolicy(_visible ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}

QScrollBar* Tree::verticalScrollBar() const
{
    return d->tree->verticalScrollBar();
}

void Tree::setHeaderVisible(bool _visible)
{
    d->tree->setHeaderHidden(!_visible);
}

void Tree::setColumnWidth(int _column, int _width)
{
    d->header->setSectionResizeMode(_column, QHeaderView::Fixed);
    d->header->resizeSection(_column, _width);
}

void Tree::setColumnVisible(int _column, bool _visible)
{
    d->header->setSectionHidden(_column, !_visible);
}

void Tree::setDragDropEnabled(bool _enabled)
{
    d->tree->setAcceptDrops(_enabled);
    d->tree->setDragEnabled(_enabled);
    d->tree->setDragDropMode(_enabled ? QAbstractItemView::DragDrop
                                      : QAbstractItemView::NoDragDrop);
    d->tree->setDropIndicatorShown(_enabled);
}

void Tree::setSelectionMode(QAbstractItemView::SelectionMode _mode)
{
    d->tree->setSelectionMode(_mode);
}

int Tree::sizeHintForColumn(int _column) const
{
    return d->tree->sizeHintForColumn(_column);
}

void Tree::setItemDelegate(QAbstractItemDelegate* _delegate)
{
    d->tree->setItemDelegate(_delegate == nullptr ? d->delegate : _delegate);
}

void Tree::setItemDelegateForColumn(int _column, QAbstractItemDelegate* _delegate)
{
    d->tree->setItemDelegateForColumn(_column, _delegate == nullptr ? d->delegate : _delegate);
}

void Tree::setCurrentIndex(const QModelIndex& _index)
{
    if (d->tree->selectionModel() != nullptr
        && d->tree->selectionModel()->selectedIndexes().contains(_index)) {
        return;
    }

    d->tree->clearSelection();
    d->tree->setCurrentIndex(_index);

    d->lastClickedIndex = _index;
}

QModelIndex Tree::currentIndex() const
{
    return d->tree->currentIndex();
}

QModelIndex Tree::indexAt(const QPoint& _pos) const
{
    return d->tree->indexAt(_pos);
}

QModelIndexList Tree::selectedIndexes() const
{
    return d->tree->selectionModel()->selectedIndexes();
}

void Tree::clearSelection()
{
    d->tree->clearSelection();
}

void Tree::setExpandsOnDoubleClick(bool _expand)
{
    d->tree->setExpandsOnDoubleClick(_expand);
}

bool Tree::isExpanded(const QModelIndex& _index) const
{
    return d->tree->isExpanded(_index);
}

void Tree::expand(const QModelIndex& _index)
{
    d->tree->expand(_index);
}

void Tree::expandAll()
{
    d->tree->expandAll();
}

void Tree::collapseAll()
{
    d->tree->collapseAll();
}

void Tree::setAutoAdjustSize(bool _auto)
{
    d->tree->setAutoAdjustSize(_auto);
}

QRect Tree::visualRect(const QModelIndex& _index) const
{
    return d->tree->visualRect(_index);
}

void Tree::setHeader(QHeaderView* _headerView)
{
    if (d->header) {
        d->header->deleteLater();
    }

    d->header = _headerView;
    d->header->setSectionResizeMode(QHeaderView::Stretch);
    d->tree->setHeader(_headerView);
}

QHeaderView* Tree::headerView() const
{
    return d->tree->header();
}

QSize Tree::viewportSizeHint() const
{
    return d->tree->viewportSizeHint();
}

void Tree::setEditTriggers(QAbstractItemView::EditTriggers _triggers)
{
    d->tree->setEditTriggers(_triggers);
}

void Tree::edit(const QModelIndex& _index)
{
    d->tree->edit(_index);
}

void Tree::restoreState(const QVariant& _state)
{
    d->tree->restoreState(_state);

    d->lastClickedIndex = currentIndex();
}

QVariant Tree::saveState() const
{
    return d->tree->saveState();
}

bool Tree::isOnItemTrilingIcon(const QPoint& _position) const
{
    const auto currentIndex = indexAt(_position);
    if (this->currentIndex() != currentIndex) {
        return false;
    }

    if (parentWidget() == nullptr) {
        return false;
    }

    if ((isLeftToRight()
         && (_position.x() < parentWidget()->width() - verticalScrollBar()->width()
                     - Ui::DesignSystem::treeOneLineItem().spacing()
                     - Ui::DesignSystem::treeOneLineItem().iconSize().width()
             || _position.x() > parentWidget()->width() - verticalScrollBar()->width()))
        || (isRightToLeft()
            && (_position.x() < verticalScrollBar()->width()
                || _position.x() > verticalScrollBar()->width()
                        + Ui::DesignSystem::treeOneLineItem().spacing()
                        + Ui::DesignSystem::treeOneLineItem().iconSize().width()))) {
        return false;
    }

    return true;
}

void Tree::processBackgroundColorChange()
{
    designSystemChangeEvent(nullptr);
}

void Tree::processTextColorChange()
{
    designSystemChangeEvent(nullptr);
}

void Tree::designSystemChangeEvent(DesignSystemChangeEvent* _event)
{
    Widget::designSystemChangeEvent(_event);

    QPalette palette = d->tree->palette();
    palette.setColor(QPalette::Base, backgroundColor());
    auto alternateBaseColor = textColor();
    alternateBaseColor.setAlphaF(Ui::DesignSystem::hoverBackgroundOpacity());
    palette.setColor(QPalette::AlternateBase, alternateBaseColor);
    palette.setColor(QPalette::Text, textColor());
    palette.setColor(QPalette::Highlight, Ui::DesignSystem::tree().selectionColor());
    palette.setColor(QPalette::HighlightedText, Ui::DesignSystem::color().accent());
    d->tree->setPalette(palette);
    d->tree->setIndentation(static_cast<int>(Ui::DesignSystem::tree().indicatorWidth()));
    auto lastDelegate = d->tree->itemDelegate();
    d->tree->setItemDelegate(nullptr);
    d->tree->setItemDelegate(lastDelegate);
    d->tree->setAnimated(isLeftToRight());

    d->header->setPalette(palette);

    d->treeScrollBar->setBackgroundColor(
        ColorHelper::transparent(textColor(), Ui::DesignSystem::elevationEndOpacity()));
    d->treeScrollBar->setHandleColor(
        ColorHelper::transparent(textColor(), Ui::DesignSystem::focusBackgroundOpacity()));
}
