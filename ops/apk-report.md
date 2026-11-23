# Отчёт сборки APK

- дата: 2026-11-23T13:39:24Z
- SDL: release-2.30.9
- NDK: 26.1.10909125
- Gradle: 8.7

## APK

```
-rw-r--r-- 1 runner runner 31956745 Nov 23 13:38 dist/cb-classic-debug.apk
e4fd64e64c6bea43cc7d4a0806a3153ca74f12675395f5944a6bc335256a4230  dist/cb-classic-debug.apk
     1056  1981-01-01 01:01   classes.dex
  1309008  1981-01-01 01:01   lib/arm64-v8a/libSDL2.so
  1330832  1981-01-01 01:01   lib/arm64-v8a/libc++_shared.so
   517416  1981-01-01 01:01   lib/arm64-v8a/libmain.so
   918044  1981-01-01 01:01   lib/armeabi-v7a/libSDL2.so
  1109732  1981-01-01 01:01   lib/armeabi-v7a/libc++_shared.so
   327812  1981-01-01 01:01   lib/armeabi-v7a/libmain.so
    43863  1981-01-01 01:01   assets/assets/project.lua
  4086673  1981-01-01 01:01   assets/assets/scenes/26_igra/016_fon/images/0222_fon.png
  4293837  1981-01-01 01:01   assets/assets/scenes/26_igra/016_fon/images/0604_fon_so_snegom.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/017_ten_igroka/images/0015_ten_igroka.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/018_ten_vraga/images/0016_ten_vraga.png
     4677  1981-01-01 01:01   assets/assets/scenes/26_igra/019_krov/images/0605_krov.png
      383  1981-01-01 01:01   assets/assets/scenes/26_igra/020_pricel/images/0018_pricel.png
      274  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0019_igrok_1.png
     5505  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0021_igrok_3.png
      196  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0022_igrok_4.png
     1011  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0023_igrok_5.png
      334  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0024_igrok_6.png
     1037  1981-01-01 01:01   assets/assets/scenes/26_igra/021_igrok/images/0026_igrok_8.png
...
    41907  1981-01-01 01:01   META-INF/MANIFEST.MF
---------                     -------
 35394311                     333 files
```

всего записей: 333


## Хвост лога сборки

```
[66/309] Generating include/SDL2/SDL_mutex.h
[67/309] Generating include/SDL2/SDL_name.h
[68/309] Generating include/SDL2/SDL_opengles2_gl2.h
[69/309] Generating include/SDL2/SDL_opengles.h
[70/309] Generating include/SDL2/SDL_opengl.h
[71/309] Generating include/SDL2/SDL_opengles2.h
[72/309] Linking C static library libcb_core.a
[73/309] Generating include/SDL2/SDL_opengles2_gl2ext.h
[74/309] Generating include/SDL2/SDL_opengles2_khrplatform.h
[75/309] Generating include/SDL2/SDL_opengles2_gl2platform.h
[76/309] Generating include/SDL2/SDL_test_assert.h
[77/309] Building C object CMakeFiles/lua54.dir/home/runner/work/CB-Classic/CB-Classic/third_party/lua-5.4/lparser.c.o
[78/309] Generating include/SDL2/SDL_pixels.h
[79/309] Generating include/SDL2/SDL_platform.h
[80/309] Generating include/SDL2/SDL_quit.h
[81/309] Generating include/SDL2/SDL_rect.h
[82/309] Generating include/SDL2/SDL_power.h
[83/309] Generating include/SDL2/SDL_render.h
[84/309] Generating include/SDL2/SDL_rwops.h
[85/309] Generating include/SDL2/SDL_scancode.h
[86/309] Generating include/SDL2/SDL_stdinc.h
[87/309] Generating include/SDL2/SDL_sensor.h
[88/309] Generating include/SDL2/SDL_shape.h
[89/309] Generating include/SDL2/SDL_surface.h
[90/309] Generating include/SDL2/SDL_system.h
[91/309] Generating include/SDL2/SDL_syswm.h
[92/309] Generating include/SDL2/SDL_test.h
[93/309] Generating include/SDL2/SDL_test_common.h
[94/309] Generating include/SDL2/SDL_test_compare.h
[95/309] Generating include/SDL2/SDL_test_crc32.h
[96/309] Generating include/SDL2/SDL_test_fuzzer.h
[97/309] Generating include/SDL2/SDL_test_font.h
[98/309] Generating include/SDL2/SDL_test_images.h
[99/309] Generating include/SDL2/SDL_test_harness.h
[100/309] Generating include/SDL2/SDL_test_memory.h
[101/309] Generating include/SDL2/SDL_test_log.h
[102/309] Generating include/SDL2/SDL_test_random.h
[103/309] Generating include/SDL2/SDL_test_md5.h
[104/309] Generating include/SDL2/SDL_touch.h
[105/309] Generating include/SDL2/SDL_types.h
[106/309] Generating include/SDL2/SDL_thread.h
[107/309] Generating include/SDL2/SDL_timer.h
[108/309] Generating include/SDL2/SDL_version.h
[109/309] Generating include/SDL2/SDL_vulkan.h
[110/309] Generating include/SDL2/begin_code.h
[111/309] Generating include/SDL2/SDL_video.h
[112/309] Generating include/SDL2/close_code.h
[113/309] Building C object sdl2/CMakeFiles/SDL2.dir/src/SDL_utils.c.o
[114/309] Building C object CMakeFiles/lua54.dir/home/runner/work/CB-Classic/CB-Classic/third_party/lua-5.4/lvm.c.o
[115/309] Linking C static library liblua54.a
[116/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp.o
FAILED: CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp.o 
/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=aarch64-none-linux-android21 --sysroot=/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/sysroot -DCB_USE_SDL2 -Dmain_EXPORTS -I/home/runner/work/CB-Classic/CB-Classic/src/c -I/home/runner/work/CB-Classic/CB-Classic/src/cpp -I/home/runner/work/CB-Classic/CB-Classic/third_party/lua-5.4 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include/SDL2 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include-config-release/SDL2 -g -DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -Wformat -Werror=format-security   -O3 -DNDEBUG  -fPIC -std=gnu++17 -MD -MT CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp.o -MF CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp.o.d -o CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp.o -c /home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp
/home/runner/work/CB-Classic/CB-Classic/src/cpp/font.cpp:8:10: fatal error: 'stb_truetype.h' file not found
#include "stb_truetype.h"
         ^~~~~~~~~~~~~~~~
1 error generated.
[117/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp.o
FAILED: CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp.o 
/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=aarch64-none-linux-android21 --sysroot=/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/sysroot -DCB_USE_SDL2 -Dmain_EXPORTS -I/home/runner/work/CB-Classic/CB-Classic/src/c -I/home/runner/work/CB-Classic/CB-Classic/src/cpp -I/home/runner/work/CB-Classic/CB-Classic/third_party/lua-5.4 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include/SDL2 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include-config-release/SDL2 -g -DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -Wformat -Werror=format-security   -O3 -DNDEBUG  -fPIC -std=gnu++17 -MD -MT CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp.o -MF CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp.o.d -o CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp.o -c /home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp
/home/runner/work/CB-Classic/CB-Classic/src/cpp/audio.cpp:15:10: fatal error: 'minimp3.h' file not found
#include "minimp3.h"
         ^~~~~~~~~~~
1 error generated.
[118/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/lua_api.cpp.o
[119/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/renderer.cpp.o
[120/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp.o
FAILED: CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp.o 
/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ --target=aarch64-none-linux-android21 --sysroot=/usr/local/lib/android/sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64/sysroot -DCB_USE_SDL2 -Dmain_EXPORTS -I/home/runner/work/CB-Classic/CB-Classic/src/c -I/home/runner/work/CB-Classic/CB-Classic/src/cpp -I/home/runner/work/CB-Classic/CB-Classic/third_party/lua-5.4 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include/SDL2 -I/home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a/sdl2/include-config-release/SDL2 -g -DANDROID -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -Wformat -Werror=format-security   -O3 -DNDEBUG  -fPIC -std=gnu++17 -MD -MT CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp.o -MF CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp.o.d -o CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp.o -c /home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp
/home/runner/work/CB-Classic/CB-Classic/src/cpp/main.cpp:666:28: error: use of undeclared identifier 'Renderer'; did you mean 'cb::Renderer'?
            app.renderer = Renderer(app.options.width, app.options.height, pixel_scale);
                           ^
/home/runner/work/CB-Classic/CB-Classic/src/cpp/renderer.hpp:14:7: note: 'cb::Renderer' declared here
class Renderer {
      ^
1 error generated.
[121/309] Building CXX object CMakeFiles/main.dir/home/runner/work/CB-Classic/CB-Classic/src/cpp/engine.cpp.o
ninja: build stopped: subcommand failed.

C++ build system [build] failed while executing:
    /usr/local/lib/android/sdk/cmake/3.22.1/bin/ninja \
      -C \
      /home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a \
      SDL2 \
      main
  from /home/runner/work/CB-Classic/CB-Classic/android/app
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt.execute(ExecuteProcess.kt:288)
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt$executeProcess$1.invoke(ExecuteProcess.kt:108)
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt$executeProcess$1.invoke(ExecuteProcess.kt:106)
	at com.android.build.gradle.internal.cxx.timing.TimingEnvironmentKt.time(TimingEnvironment.kt:32)
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt.executeProcess(ExecuteProcess.kt:106)
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt.executeProcess$default(ExecuteProcess.kt:85)
	at com.android.build.gradle.internal.cxx.build.CxxRegularBuilder.executeProcessBatch(CxxRegularBuilder.kt:332)
	at com.android.build.gradle.internal.cxx.build.CxxRegularBuilder.build(CxxRegularBuilder.kt:129)
	at com.android.build.gradle.tasks.ExternalNativeBuildTask$doTaskAction$$inlined$recordTaskAction$1.invoke(BaseTask.kt:70)
	at com.android.build.gradle.internal.tasks.Blocks.recordSpan(Blocks.java:51)
	at com.android.build.gradle.tasks.ExternalNativeBuildTask.doTaskAction(ExternalNativeBuildTask.kt:145)
	at com.android.build.gradle.internal.tasks.UnsafeOutputsTask$taskAction$$inlined$recordTaskAction$1.invoke(BaseTask.kt:65)
	at com.android.build.gradle.internal.tasks.Blocks.recordSpan(Blocks.java:51)
	at com.android.build.gradle.internal.tasks.UnsafeOutputsTask.taskAction(UnsafeOutputsTask.kt:63)
	at java.base/jdk.internal.reflect.NativeMethodAccessorImpl.invoke0(Native Method)
	at java.base/jdk.internal.reflect.NativeMethodAccessorImpl.invoke(NativeMethodAccessorImpl.java:77)
	at java.base/jdk.internal.reflect.DelegatingMethodAccessorImpl.invoke(DelegatingMethodAccessorImpl.java:43)
	at org.gradle.internal.reflect.JavaMethod.invoke(JavaMethod.java:125)
	... 119 more
Caused by: com.android.ide.common.process.ProcessException: Error while executing process /usr/local/lib/android/sdk/cmake/3.22.1/bin/ninja with arguments {-C /home/runner/work/CB-Classic/CB-Classic/android/app/.cxx/Release/601fi4a6/arm64-v8a SDL2 main}
	at com.android.build.gradle.internal.process.GradleProcessResult.buildProcessException(GradleProcessResult.java:73)
	at com.android.build.gradle.internal.process.GradleProcessResult.assertNormalExitValue(GradleProcessResult.java:48)
	at com.android.build.gradle.internal.cxx.process.ExecuteProcessKt.execute(ExecuteProcess.kt:277)
	... 136 more
Caused by: org.gradle.process.internal.ExecException: Process 'command '/usr/local/lib/android/sdk/cmake/3.22.1/bin/ninja'' finished with non-zero exit value 1
	at org.gradle.process.internal.DefaultExecHandle$ExecResultImpl.assertNormalExitValue(DefaultExecHandle.java:442)
	at com.android.build.gradle.internal.process.GradleProcessResult.assertNormalExitValue(GradleProcessResult.java:46)
	... 137 more


BUILD FAILED in 53s
29 actionable tasks: 29 executed
код возврата gradle: 1
СБОРКА НЕ УДАЛАСЬ
```
