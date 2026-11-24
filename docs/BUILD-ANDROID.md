# Сборка APK (Android)

Порт целиком написан на C, C++ и Lua, поэтому на телефоне работает тот же движок,
что и на десктопе: SDL2 даёт окно и касания, Lua крутит логику проекта, C-часть
рисует PNG.

## Готовый APK

Отладочная сборка публикуется автоматически:

**https://github.com/ehekufa/CB-Classic/releases/tag/apk-latest** → `cb-classic-debug.apk`

- `applicationId` `org.cbclassic`, версия `2.8`
- `minSdk 21` (Android 5.0), `targetSdk 34`
- ABI: `arm64-v8a` и `armeabi-v7a` (x86 не собирается — на эмуляторах работает через
  трансляцию ARM, если она включена)
- подпись — стандартный отладочный ключ, поэтому система попросит разрешить
  установку из неизвестных источников

Тот же файл лежит прямо в ветке: [`dist/cb-classic-debug.apk`](../dist/cb-classic-debug.apk)
(в git попадает только потому, что `objects.githubusercontent.com`, откуда GitHub отдаёт
release-ассеты, в России часто заблокирован — обычный файл репозитория качается надёжнее).
sha256 последней сборки — в `ops/apk-report.md`.
- внутри APK лежат все 122 PNG, 16 MP3, `project.lua`, `layout.lua`, `index.json`,
  136 сгенерированных Lua-модулей и три файла `src/lua` — ничего не докачивается из сети

Отчёт каждой сборки (размер APK, sha256, sha, содержимое, хвост лога Gradle) лежит
в `ops/apk-report.md` — его пишет тот же прогон CI, что и собирает APK.

## Как устроено

```
android/app/src/main/java/org/cbclassic/MainActivity.java   точка входа, распаковка ассетов
android/CMakeLists.txt                                      сборка libmain.so (NDK)
android/app/build.gradle                                    ABI, minSdk, assets
.github/workflows/android.yml                               сборка APK на раннере
```

1. `MainActivity` наследует `org.libsdl.app.SDLActivity` (Java-классы SDL2 кладёт в проект CI
   из `third_party/SDL/android-project`).
2. При первом запуске ассеты из APK распаковываются в `filesDir/root/` — дальше движок
   читает обычные файлы, ровно как на десктопе (`--root`).
3. `libmain.so` содержит движок и вызывает `SDL_main`; `main()` на Android берёт корень
   из `SDL_AndroidGetInternalStoragePath() + "/root"` и стартует со сцены 42 «Загрузка».
4. Касания (`SDL_FINGERDOWN/MOTION/UP`) переводятся в координаты сцены
   (1500×720, начало координат в центре) и попадают в тот же `Engine::set_touch`,
   что и мышь на десктопе.

## Сборка вручную

Нужны JDK 17, Android SDK (`platforms;android-34`, `build-tools;34.0.0`,
`cmake;3.22.1`), NDK `26.1.10909125` и исходники SDL2.

```bash
# 1. SDL2 (в git не хранится)
curl -sSL -o /tmp/sdl.tgz https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.9.tar.gz
mkdir -p third_party/SDL && tar xzf /tmp/sdl.tgz -C third_party/SDL --strip-components=1

# 2. Java-классы SDL и ресурсы
mkdir -p android/app/src/main/java/org/libsdl/app
cp third_party/SDL/android-project/app/src/main/java/org/libsdl/app/*.java \
   android/app/src/main/java/org/libsdl/app/
mkdir -p android/app/src/main/assets/src android/app/src/main/assets/generated
cp -r assets         android/app/src/main/assets/assets
cp -r src/lua        android/app/src/main/assets/src/lua
cp -r generated/lua  android/app/src/main/assets/generated/lua

# 3. сборка
echo "sdk.dir=$ANDROID_HOME" > android/local.properties
cd android && gradle assembleDebug
# APK: android/app/build/outputs/apk/debug/app-debug.apk
```

В репозитории нет `gradlew`/`gradle-wrapper.jar`: CI скачивает дистрибутив Gradle 8.7
сам, а Android Studio подставит свой.

## Что учесть на телефоне

- Ориентация зафиксирована горизонтальная, интерфейс полноэкранный — как в оригинале.
- Касание — один палец; джойстик, кнопки атаки и меню работают, но одновременное
  нажатие двух кнопок пока не поддержано (см. ограничения в README).
- Звука нет ни на Android, ни на десктопе: MP3-декодера в порте нет.
- Логи: `adb logcat -s cbclassic SDL`.
