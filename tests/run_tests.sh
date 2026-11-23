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
grep -q "Кубки: 5" /tmp/cb_cups2.log
check $?

step "звук: эффекты проекта звучат, музыка синтезируется"
$BIN --frames 30 --scene 26 --script tests/scripts/joystick.txt \
     --profile "61=Гость,62=123,60=3,3=100,4=6,12=1" --out /tmp/cb_frames_snd \
     >/tmp/cb_snd.log 2>&1
check $?
grep -Eq "прозвучало звуков: [1-9]" /tmp/cb_snd.log
check $?
$BIN --export-music /tmp/cb_theme.wav >/tmp/cb_music.log 2>&1
check $?
python3 - <<'PYW'
import sys, wave
try:
    w = wave.open("/tmp/cb_theme.wav")
except Exception:
    sys.exit(1)
sys.exit(0 if w.getnframes() > 44100 * 10 else 1)
PYW
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
