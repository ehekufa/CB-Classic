#!/bin/sh
# Самотесты порта: C-часть (PNG/inflate), конвертер, Lua-синтаксис, движок.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
BIN=build/cb-classic
FAILED=0

step() { printf '\n== %s ==\n' "$1"; }
check() {
  if [ "$1" -eq 0 ]; then
    printf '   OK\n'
  else
    printf '   ПРОВАЛ\n'
    FAILED=$((FAILED + 1))
  fi
}

step "сборка"
make -s -j4
check $?

step "C: PNG-декодер на всех ассетах + round-trip"
cc -O2 -std=c11 -Isrc/c tests/c/test_png.c src/c/png.c src/c/inflate.c -o build/test_png
./build/test_png assets
check $?

step "инструменты: извлечение ассетов идемпотентно"
python3 tools/extract_assets.py --src original --out /tmp/cb_assets_check >/tmp/cb_extract.log 2>&1
check $?
grep -q "PNG: 153" /tmp/cb_extract.log
check $?

step "инструменты: конвертер покрывает все блоки"
python3 tools/catrobat2lua.py --src original --out /tmp/cb_lua_check >/tmp/cb_convert.log 2>&1
check $?
grep -q "неподдержанных блоков: 0" /tmp/cb_convert.log
check $?

step "Lua: синтаксис сгенерированных модулей"
$BIN --check generated/lua
check $?

step "движок: сцена «Загрузка» (42) стартует и переходит дальше"
$BIN --frames 40 --scene 42 --out /tmp/cb_frames_42 >/tmp/cb_run42.log 2>&1
check $?
grep -q "сцена: 42" /tmp/cb_run42.log
check $?

step "движок: меню (1) рисует спрайты"
$BIN --frames 10 --scene 1 --out /tmp/cb_frames_1 --verbose >/tmp/cb_run1.log 2>&1
check $?
grep -q "спрайтов на сцене: 10" /tmp/cb_run1.log
check $?

step "движок: игра (26) с профилем и вводом джойстика"
$BIN --frames 60 --scene 26 --script tests/scripts/joystick.txt \
     --profile "61=Гость,62=123,60=3,3=100,4=6,12=1" --out /tmp/cb_frames_26 >/tmp/cb_run26.log 2>&1
check $?
grep -q "ошибок: 0" /tmp/cb_run26.log
check $?

step "управление: джойстик ведёт игрока вправо, а не вверх"
$BIN --frames 40 --scene 26 --script tests/scripts/joystick_move.txt \
     --profile "61=Гость,62=123,60=3,3=100,4=6,12=1" --out /tmp/cb_frames_joy --verbose \
     >/tmp/cb_joy.log 2>&1
check $?
awk '/Игрок /{print $3}' /tmp/cb_joy.log | tail -1 | \
  python3 -c "import sys; sys.exit(0 if float(sys.stdin.read().strip() or 0) > 100 else 1)"
check $?

step "способности: кнопка «Супер атака» перезаряжается (своя пауза у всех классов)"
$BIN --frames 520 --scene 26 --profile "61=Гость,62=123,60=3,3=100,4=600,12=1,47=2" \
     --set-var "65=1@2" --script tests/scripts/super_attack.txt --trace 26 --bench 520 --no-sound \
     >/tmp/cb_cooldown.log 2>&1
check $?
grep -q "ярк= 85.0" /tmp/cb_cooldown.log
check $?
# 15 секунд при 30 fps — это ~450 серых кадров; нажатие во время паузы её не продлевает
DIM=$(grep -c "ярк= 85.0" /tmp/cb_cooldown.log || true)
if [ "$DIM" -ge 430 ] && [ "$DIM" -le 470 ]; then
  printf '   OK (серых кадров %s ≈ 15 с)\n' "$DIM"
else
  printf '   ПРОВАЛ: серых кадров %s, ждали ~450 (15 с)\n' "$DIM"
  FAILED=$((FAILED + 1))
fi
LAST=$(grep "id=26" /tmp/cb_cooldown.log | tail -1)
case "$LAST" in
  *"ярк=100.0"*) printf '   OK кнопка снова яркая\n' ;;
  *) printf '   ПРОВАЛ: кнопка осталась серой\n'; FAILED=$((FAILED + 1)) ;;
esac

step "атака: прицел держится, пока палец на кнопке А, и гаснет после отпускания"
$BIN --frames 60 --scene 26 --profile "61=Гость,62=123,60=3,3=100,4=6,12=1,47=4" \
     --script tests/scripts/attack_hold.txt --trace 20 --no-sound \
     >/tmp/cb_aim.log 2>&1
check $?
grep -q "trace   20 .*vis=1" /tmp/cb_aim.log
check $?
grep -q "trace   50 .*vis=0" /tmp/cb_aim.log
check $?

step "валентинки: счётчик поверх интерфейса и сохранение между сценами"
rm -f saves/db.tsv
$BIN --frames 12 --scene 26 --profile "61=Гость,62=123,60=3,3=100,4=6,12=1,47=4" \
     --set-var "93=12@5" --out /tmp/cb_frames_val --no-sound >/tmp/cb_val1.log 2>&1
check $?
grep -q "var:93	12\|var:93 12" saves/db.tsv
check $?
$BIN --frames 6 --scene 1 --out /tmp/cb_frames_val2 --verbose >/tmp/cb_val2.log 2>&1
check $?
grep -q 'hud:валентинки .*значение="Валентинки: 12"' /tmp/cb_val2.log
check $?
# надпись оригинала под «Конфетой» больше не показывается — счётчик один
grep -q 'var:92 .*видно=0' /tmp/cb_val2.log
check $?

step "скины: выбор скина меняет картинку игрока"
$BIN --frames 40 --scene 95 --script tests/scripts/skin_select.txt \
     --out /tmp/cb_frames_skin --verbose >/tmp/cb_skin.log 2>&1
check $?
grep -q "Игрок 1 .*0931_" /tmp/cb_skin.log
check $?

step "кубки: победа даёт кубки, счёт сохраняется и виден в меню"
rm -f saves/db.tsv
$BIN --frames 30 --scene 26 --set-var "4=0@15" --out /tmp/cb_frames_cups --verbose \
     >/tmp/cb_cups.log 2>&1
check $?
grep -q "toast: Победа! +5 кубков" /tmp/cb_cups.log
check $?
grep -q "cups.5" saves/db.tsv
check $?
$BIN --frames 3 --scene 1 --out /tmp/cb_frames_cups2 --verbose >/tmp/cb_cups2.log 2>&1
grep -q 'var:91 .*значение="5"' /tmp/cb_cups2.log
check $?
# счёт стоит в жёлтой полосе панели, отдельного HUD в углу больше нет
grep -q "кубки-счёт" /tmp/cb_cups2.log && CUPS_HUD=1 || CUPS_HUD=0
if [ "$CUPS_HUD" -eq 0 ]; then
  printf '   OK\n'
else
  printf '   ПРОВАЛ: HUD кубков в углу остался\n'
  FAILED=$((FAILED + 1))
fi

step "шрифт: используется Roboto из third_party/fonts"
$BIN --frames 2 --scene 1 --out /tmp/cb_frames_font --verbose >/tmp/cb_font.log 2>&1
check $?
grep -q "шрифт: .*Roboto-Regular.ttf" /tmp/cb_font.log
check $?

step "звук: играют эффекты проекта, фоновой музыки нет"
$BIN --frames 30 --scene 26 --script tests/scripts/joystick.txt \
     --profile "61=Гость,62=123,60=3,3=100,4=6,12=1" --out /tmp/cb_frames_snd \
     >/tmp/cb_snd.log 2>&1
check $?
grep -Eq "прозвучало звуков: [1-9]" /tmp/cb_snd.log
check $?
# музыки быть не должно: ни синтезатора в коде, ни флагов в CLI
if grep -rq "build_music\|start_music\|export_music" src/cpp; then
  printf '   ПРОВАЛ: в коде осталась музыка\n'
  FAILED=$((FAILED + 1))
else
  printf '   OK\n'
fi
$BIN --help >/tmp/cb_help.log 2>&1
if grep -q -- "--export-music" /tmp/cb_help.log; then
  printf '   ПРОВАЛ: флаг музыки остался в CLI\n'
  FAILED=$((FAILED + 1))
else
  printf '   OK\n'
fi

step "звук: нажатие кнопки меню щёлкает звуком из проекта"
printf 'f2: tap -400 40\n' > /tmp/cb_click.txt
$BIN --frames 8 --scene 1 --script /tmp/cb_click.txt --out /tmp/cb_frames_click \
     >/tmp/cb_click.log 2>&1
check $?
grep -Eq "прозвучало звуков: [1-9]" /tmp/cb_click.log
check $?

step "адаптация: суперсэмплинг сохраняет раскладку (2x кадр)"
$BIN --frames 3 --scene 77 --pixel-scale 2 --out /tmp/cb_frames_x2 >/tmp/cb_x2.log 2>&1
check $?
python3 - <<'PYW'
import struct, sys, glob
files = sorted(glob.glob("/tmp/cb_frames_x2/*.png"))
if not files:
    sys.exit(1)
data = open(files[-1], "rb").read(33)
w, h = struct.unpack(">II", data[16:24])
sys.exit(0 if (w, h) == (2696, 1440) else 1)
PYW
check $?

step "плавность: время кадра измеряется (--bench, без записи PNG)"
$BIN --scene 26 --bench 12 --no-sound >/tmp/cb_bench.log 2>&1
check $?
grep -Eq "бенч: .*fps" /tmp/cb_bench.log
check $?

step "ввод: все кнопки проекта получают нажатие"
$BIN --check-buttons >/tmp/cb_buttons.log 2>&1
check $?
grep -q "не получают нажатие: 0" /tmp/cb_buttons.log
check $?

step "ввод: текстовые поля регистрации (77) принимают ввод с клавиатуры"
$BIN --frames 24 --scene 77 --script tests/scripts/input_registration.txt \
     --out /tmp/cb_frames_77 --verbose >/tmp/cb_run77.log 2>&1
check $?
grep -q 'ввод подтверждён: Имя = \[Гость\]' /tmp/cb_run77.log
check $?
grep -q 'ввод подтверждён: Пароль = \[12345\]' /tmp/cb_run77.log
check $?
grep -q "сцена: 1 (Меню)" /tmp/cb_run77.log
check $?

step "ввод: кнопки меню переключают сцены"
$BIN --frames 12 --scene 1 --script tests/scripts/menu_buttons.txt \
     --out /tmp/cb_frames_menu >/tmp/cb_menu.log 2>&1
check $?
grep -q "сцена: 70 (Режимы)" /tmp/cb_menu.log
check $?

step "ввод: блок «спросить» (ask) возвращает ответ"
$BIN --frames 20 --scene 1 --script tests/scripts/ask_reset.txt \
     --out /tmp/cb_frames_ask --verbose >/tmp/cb_ask.log 2>&1
check $?
grep -q '= \[abc\]' /tmp/cb_ask.log
check $?

step "ввод: два пальца одновременно (джойстик + кнопка) не ломают игру"
$BIN --frames 30 --scene 26 --script tests/scripts/multitouch.txt \
     --profile "61=Гость,62=123,60=3,3=100,4=6,12=1" --out /tmp/cb_frames_mt >/tmp/cb_mt.log 2>&1
check $?
grep -q "ошибок: 0" /tmp/cb_mt.log
check $?

step "сеть: без --fb-url сеть выключена (игра работает офлайн)"
$BIN --frames 5 --scene 1 --out /tmp/cb_frames_offline --verbose >/tmp/cb_offline.log 2>&1
check $?
grep -q "сеть: выключена" /tmp/cb_offline.log
check $?

step "сеть: Firebase RTDB — чтение базы игрой (эмулятор базы)"
FB_PORT=8899
cat > /tmp/cb_fb_seed.json <<'EOF'
{"Гость": {"123": {"К": 777, "В": 0}}, "world": {"X1": 42, "Y1": -7, "Ивент": "тест"}}
EOF
rm -f /tmp/cb_fb_dump.json
python3 tests/firebase_mock.py --port "$FB_PORT" --seed /tmp/cb_fb_seed.json \
    --dump /tmp/cb_fb_dump.json >/tmp/cb_fb_mock.log 2>&1 &
FB_PID=$!
sleep 1
mv saves/db.tsv /tmp/cb_db_backup.tsv 2>/dev/null || true
$BIN --scene 1 --frames 45 --fps 30 --realtime --out /tmp/cb_frames_fb \
     --profile "61=Гость,62=123" --fb-url "http://127.0.0.1:$FB_PORT/" --verbose \
     >/tmp/cb_fb_read.log 2>&1
check $?
grep -q "Гость/123/К=777" /tmp/cb_fb_read.log
check $?

step "сеть: запись в базу (промокод «Lover» -> валентинки на сервере)"
$BIN --scene 31 --frames 60 --fps 30 --realtime --out /tmp/cb_frames_fb31 \
     --profile "61=Гость,62=123" --set-var "81=Lover@3" --script tests/scripts/fb_promo.txt \
     --fb-url "http://127.0.0.1:$FB_PORT/" >/tmp/cb_fb_write.log 2>&1
check $?
python3 tests/fb_check_write.py /tmp/cb_fb_dump.json "Гость/123/В" 20
check $?
kill "$FB_PID" 2>/dev/null || true
mv /tmp/cb_db_backup.tsv saves/db.tsv 2>/dev/null || true

step "движок: кадры — валидные PNG нужного размера"
python3 - <<'PY'
import struct, sys, glob
ok = True
for path in glob.glob("/tmp/cb_frames_1/*.png")[:3]:
    data = open(path, "rb").read(33)
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        ok = False
    w, h = struct.unpack(">II", data[16:24])
    if (w, h) != (1348, 720):
        ok = False
        print("неверный размер кадра:", path, w, h)
sys.exit(0 if ok else 1)
PY
check $?

printf '\n'
if [ "$FAILED" -eq 0 ]; then
  echo "все проверки пройдены"
else
  echo "провалов: $FAILED"
fi
exit $FAILED
