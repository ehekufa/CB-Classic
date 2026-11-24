// native_ui.hpp — системные окна Android: Toast и AlertDialog с EditText.
//
// Движок в остальном рисует интерфейс сам, но «всплывающие окна» на телефоне
// должны быть настоящими андроидными: там своя клавиатура, выделение и удаление
// текста, системное затемнение и кнопки. На десктопе всё это недоступно —
// функции просто ничего не делают, и игра рисует карточку как раньше.
#ifndef CB_NATIVE_UI_HPP
#define CB_NATIVE_UI_HPP

#include <string>

namespace cb {

// Есть ли живая активность Android (на десктопе всегда false).
bool native_ui_available();

// Показать системное уведомление внизу экрана.
void native_toast(const std::string &text);

// Показать модальное окно с полем ввода. Ответ придёт в очередь,
// его забирает native_ask_result (колбэк приходит с UI-потока).
void native_ask(const std::string &title, const std::string &hint,
                const std::string &initial, const std::string &ok,
                const std::string &cancel);

// Убрать открытое окно (игра закрыла вопрос).
void native_ask_dismiss();

// Забрать ответ: text — что ввели, accepted — нажата ли «Принять».
// false — ответа пока нет.
bool native_ask_result(std::string *text, bool *accepted);

}  // namespace cb

#endif  // CB_NATIVE_UI_HPP
