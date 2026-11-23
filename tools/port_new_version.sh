#!/bin/sh
# port_new_version.sh — перенести новую версию проекта Pocket Code в порт.
#
#   tools/port_new_version.sh super.kortohka.zip
#   tools/port_new_version.sh super.kortohka.zip --no-build
#
# Что делает по шагам:
#   1) распаковывает архив новой версии (zip / .catrobat; можно и готовую папку);
#   2) находит корень экспорта Pocket Code (каталог с options.txt и scene_*);
#   3) заменяет original/ свежими данными; файлы больше 50 МБ пропускает —
#      такие в репозиторий не влезают (они фиксируются в ops/excluded-large-files.txt);
#   4) пересобирает assets/ (tools/extract_assets.py) и generated/lua
#      (tools/catrobat2lua.py);
#   5) собирает движок и прогоняет самотесты (--no-build — пропустить).
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SRC=${1:-}
BUILD=1
if [ "${2:-}" = "--no-build" ]; then
  BUILD=0
fi

if [ -z "$SRC" ]; then
  echo "использование: tools/port_new_version.sh <архив.zip|папка> [--no-build]" >&2
  exit 2
fi
if [ ! -e "$SRC" ]; then
  echo "нет такого файла/папки: $SRC" >&2
  exit 2
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

if [ -d "$SRC" ]; then
  PROJ=$SRC
else
  echo "== распаковка $(basename "$SRC") =="
  unzip -q -o "$SRC" -d "$TMP/unpack" || {
    echo "не удалось распаковать $SRC (нужен zip/catrobat)" >&2
    exit 1
  }
  PROJ=$TMP/unpack
fi

# Корень экспорта: каталог, где лежат options.txt/counter.txt и папки scene_*.
find_root() {
  for candidate in "$1" "$1"/*; do
    if [ -d "$candidate" ] && [ -f "$candidate/options.txt" ]; then
      echo "$candidate"
      return 0
    fi
  done
  found=$(find "$1" -maxdepth 3 -name options.txt -print 2>/dev/null | head -1)
  if [ -n "$found" ]; then
    dirname "$found"
    return 0
  fi
  return 1
}

if ! PROJ_ROOT=$(find_root "$PROJ"); then
  echo "внутри не нашёлся экспорт Pocket Code (options.txt + scene_*)" >&2
  echo "что было в архиве:" >&2
  find "$PROJ" -maxdepth 2 | head -20 >&2
  exit 1
fi
echo "== проект: $PROJ_ROOT =="
echo "   сцен: $(find "$PROJ_ROOT" -maxdepth 1 -type d -name 'scene_*' | wc -l)"

# Старые данные заменяем целиком: иначе удалённые в новой версии объекты
# остались бы в assets/ и в сгенерированных Lua-модулях.
rm -rf "$ROOT/original"
mkdir -p "$ROOT/original"
( cd "$PROJ_ROOT" && find . -type f ! -size +50M -exec cp -p --parents {} "$ROOT/original/" \; )
SKIPPED=$(cd "$PROJ_ROOT" && find . -type f -size +50M | wc -l)
if [ "$SKIPPED" -gt 0 ]; then
  echo "   пропущено файлов больше 50 МБ: $SKIPPED (в репозиторий не кладём)"
  mkdir -p "$ROOT/ops"
  : > "$ROOT/ops/excluded-large-files.txt"
  ( cd "$PROJ_ROOT" && find . -type f -size +50M | sort ) | while IFS= read -r big; do
    rel=${big#./}
    size=$(cd "$PROJ_ROOT" && stat -c%s "$big")
    sum=$(cd "$PROJ_ROOT" && sha256sum "$big" | cut -d' ' -f1)
    printf '%s\t%s\t%s\n' "$rel" "$size" "$sum" | tee -a "$ROOT/ops/excluded-large-files.txt"
  done
else
  : > "$ROOT/ops/excluded-large-files.txt"
fi

echo "== assets =="
# Старые спрайты/иконки удаляем: в новой версии сцены переименованы
# (было 01_menyu/281_konfeta — стало 281_yayca), и без очистки в APK
# уезжали картинки и звуки уже не существующих объектов.
rm -rf "$ROOT/assets/scenes" "$ROOT/assets/icons"
python3 "$ROOT/tools/extract_assets.py" --src "$ROOT/original" --out "$ROOT/assets"
echo "== generated/lua =="
python3 "$ROOT/tools/catrobat2lua.py" --src "$ROOT/original" --out "$ROOT/generated"

# Версия ассетов в APK: без неё телефон не перезапишет распакованные файлы.
MAIN="$ROOT/android/app/src/main/java/org/cbclassic/MainActivity.java"
if [ -f "$MAIN" ]; then
  OLD_VER=$(sed -n 's/.*ASSETS_VERSION = \([0-9]*\);.*/\1/p' "$MAIN" | head -1)
  if [ -n "$OLD_VER" ]; then
    NEW_VER=$((OLD_VER + 1))
    sed -i "s/ASSETS_VERSION = $OLD_VER;/ASSETS_VERSION = $NEW_VER;/" "$MAIN"
    echo "== ASSETS_VERSION: $OLD_VER -> $NEW_VER =="
  fi
fi

if [ "$BUILD" -eq 1 ]; then
  echo "== сборка =="
  make -C "$ROOT" -s -j4
  echo "== самотесты =="
  "$ROOT/tests/run_tests.sh"
fi

echo
echo "== что изменилось =="
git -C "$ROOT" status --short | head -30
echo
echo "дальше: поднять ASSETS_VERSION в android/app/src/main/java/org/cbclassic/MainActivity.java,"
echo "закоммитить и запушить — APK соберётся в Actions."
