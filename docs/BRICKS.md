# Покрытие блоков Pocket Code

Сгенерировано `tools/report.py` по данным `original/`.

Блоков в проекте: **1776**, различных видов: **74**.

## Блоки

| блок | раз | во что превращается | статус |
|---|---:|---|---|
| `wait` | 199 | E.wait | готово |
| `if` | 120 | if/else | готово |
| `endIf` | 120 | структурный: закрывает if | готово |
| `function` | 102 | E.call_function | готово (получателей нет) |
| `setBrightness` | 93 | E.set_bright | готово |
| `setPosition` | 85 | E.set_pos | готово |
| `hide` | 77 | E.hide | готово |
| `setSize` | 76 | E.set_size | готово |
| `setVariable` | 73 | E.set_gv / E.set_lv | готово |
| `cycleForever` | 51 | while true + E.frame() | готово |
| `endCycleForever` | 51 | структурный: закрывает forever | готово |
| `runScene` | 44 | E.run_scene | готово |
| `setRotate` | 43 | E.set_rot | готово |
| `fbSetValue` | 42 | E.fb_set (локальная эмуляция) | готово |
| `endRepeat` | 40 | структурный: закрывает repeat / repeatIsTrue | готово |
| `fbGetValue` | 34 | E.fb_get (локальная эмуляция) | готово |
| `removeObjectCamera` | 33 | E.camera_off | готово |
| `show` | 32 | E.show | готово |
| `else` | 29 | встроено в if | готово |
| `setImageToName` | 26 | E.set_image_by_id | готово |
| `goSteps` | 25 | E.go_steps | готово |
| `repeatIsTrue` | 22 | while условие + E.frame() | готово |
| `editPositionY` | 22 | E.add_y | готово |
| `broadcastFun>object` | 22 | E.broadcast_to_object | готово (получателей нет) |
| `setPositionY` | 21 | E.set_y | готово |
| `showToast` | 20 | E.toast | готово |
| `goTo` | 20 | E.go_to | готово |
| `addBody` | 19 | E.add_body (журнал) | заглушка: нет физики |
| `broadcastFunction` | 19 | E.broadcast | готово (получателей в проекте нет) |
| `repeat` | 18 | for + E.frame() | готово |
| `readVariable` | 16 | E.load_var | готово |
| `editVariable` | 16 | E.add_gv / E.add_lv | готово |
| `saveVariable` | 15 | E.store_var (локальное хранилище) | готово |
| `removeAdaptiveSizeDevice` | 13 | E.adaptive_off | готово |
| `deleteClone` | 10 | E.delete_clone | готово |
| `clone` | 9 | E.clone | готово |
| `setAlpha` | 8 | E.set_alpha | готово |
| `editSize` | 7 | E.add_size | готово |
| `transitionPosition2` | 7 | E.transition (плавный ход) | готово |
| `isSensor` | 7 | E.set_sensor (журнал) | заглушка: нет физики |
| `showVariable2` | 6 | E.text_show_var | готово |
| `ask` | 6 | E.ask | готово: поле ввода + ожидание ответа |
| `setImageToId` | 6 | E.set_image_by_id | готово |
| `exitGame` | 5 | E.exit_game | готово |
| `editRotateRight` | 5 | E.add_rot | готово |
| `deleteTextField` | 5 | E.text_delete | готово |
| `editAlpha` | 4 | E.add_alpha | готово |
| `editPositionX` | 4 | E.add_x | готово |
| `createRadialParticle` | 4 | E.particles_create | заглушка: нет частиц |
| `setPositionParticle` | 4 | E.particles_move | заглушка: нет частиц |
| `setBackgroundColorHex` | 3 | E.set_bg_color | готово |
| `setPositionX` | 3 | E.set_x | готово |
| `waitIfTrue2` | 3 | E.wait_until | готово |
| `setQuareWHHitbox` | 3 | E.set_hitbox (журнал) | заглушка: нет физики |
| `setRotateToObject` | 3 | E.face | готово |
| `playSound` | 3 | E.play_sound (журнал) | частично: без MP3-декодера |
| `createTextField` | 3 | E.text_create | готово: ввод с клавиатуры |
| `setPositionTextField` | 3 | E.text_set('pos') | готово |
| `broadcastFun>allObjects` | 2 | E.broadcast | готово (получателей нет) |
| `setGravityAllObjects` | 2 | E.set_gravity (журнал) | заглушка: нет физики |
| `nextImage` | 2 | E.next_image | готово |
| `playSoundAndWait` | 2 | E.play_sound_wait | частично: без MP3-декодера |
| `addElementArray` | 2 | E.array_add | готово |
| `placeholderTextField` | 2 | E.text_set('placeholder') | готово |
| `setAlignTextField` | 2 | E.text_set('align') | готово |
| `openLink` | 1 | E.open_link (журнал) | готово |
| `setLayer` | 1 | E.set_layer | готово |
| `focusCameraToObject` | 1 | E.camera_focus (журнал) | заглушка |

## Выражения и сенсоры

| конструкция | реализация |
|---|---|
| number | числовой литерал (склейка кусков: 3,5,0 → 350) |
| text | строковый литерал |
| globalVariable | E.gv(id) |
| localVariable | E.lv(o, id) |
| globalArray | E.array(id) |
| function: =, ≠, <, >, ≤, ≥ | E.op_eq / op_ne / op_lt / op_gt / op_le / op_ge |
| function: +, -, ×, ÷ | E.op_add / op_sub / op_mul / op_div |
| function: and, or, not | E.truthy(...) |
| function: positionX / positionY | E.pos_x(o) / E.pos_y(o) |
| function: rotation / size / brightness / alpha | геттеры спрайта |
| function: numberImage / nameImage | номер и имя текущей картинки |
| function: touchDisplayX/Y/Id | координаты касания |
| function: touchesObject2(id) | E.touching_object(o, id) |
| function: random(a,b) | E.random |
| function: connect2(a,b,c) | E.fb_key (путь в локальном хранилище) |
| ref: scenes/objects/images/sounds | числовой id из проекта |
| alignText / GL / typeBody / onOrOff | строковый/служебный токен |

## Итог

* блоков без реализации: **0**
* заглушки (пишут в журнал, но не моделируют физику/частицы): `addBody`, `isSensor`, `setGravityAllObjects`, `setQuareWHHitbox`, `createRadialParticle`, `setPositionParticle`, `focusCameraToObject`.
* звук: выбор и запуск звука перенесены, воспроизведения нет (нет MP3-декодера).
