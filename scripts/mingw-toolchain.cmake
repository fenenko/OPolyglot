get_filename_component(MINGW_ROOT "../../../build/mingw64" REALPATH)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
message(STATUS "--------START---------")
message(STATUS	"MINGW PATH ${MINGW_ROOT}")
set(CMAKE_FIND_ROOT_PATH ${MINGW_ROOT} /usr/x86_64-w64-mingw32)
message(STATUS	"ROOT PATH ${CMAKE_FIND_ROOT_PATH}")

# Це критично: кажемо CMake не "придумувати" прапорці від Linux
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Пропускаємо перевірку лінкера в тестах
#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(PKG_CONFIG_EXECUTABLE x86_64-w64-mingw32-pkg-config CACHE BRAND "pkg-config executable")

# Або, якщо спеціального бінарника немає, змушуємо звичайний pkg-config шукати в папках MinGW:
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} "${MINGW_ROOT}/lib/pkgconfig:/usr/x86_64-w64-mingw32/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "${MINGW_ROOT}")
