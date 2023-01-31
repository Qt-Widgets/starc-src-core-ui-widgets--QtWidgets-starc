#pragma once

#include "standard_key_handler.h"


namespace KeyProcessingLayer {
/**
 * @brief Класс выполняющий обработку нажатия клавиш в блоке описание действия
 */
class DescriptionHandler : public StandardKeyHandler
{
public:
    explicit DescriptionHandler(Ui::ComicBookTextEdit* _editor);

protected:
    /**
     * @brief Реализация интерфейса AbstractKeyHandler
     */
    /** @{ */
    void handleEnter(QKeyEvent* _event = 0);
    void handleTab(QKeyEvent* _event = 0);
    void handleOther(QKeyEvent* _event = 0);
    /** @} */
};
} // namespace KeyProcessingLayer
