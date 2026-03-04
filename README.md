# Windows C++ Camera Application

Современное приложение для работы с камерой на Windows с использованием Direct2D для рендеринга и Media Foundation для работы с камерой.

## Особенности

- **Современный UI**: Использует Direct2D для плавного рендеринга
- **Работа с камерой**: Интеграция с Windows Media Foundation
- **Анимированное меню**: Интерактивное меню с анимациями
- **Режим отладки**: Расширенная отладочная информация в debug режиме
- **HiDPI поддержка**: Корректное отображение на экранах с высоким разрешением

## Системные требования

- Windows 10 или новее
- Visual Studio 2019+ или MinGW-w64
- Direct2D, DirectWrite, Media Foundation (входят в состав Windows)

## Сборка и запуск

### Автоматическая сборка

Используйте скрипт `build.sh` для автоматической сборки:

```bash
# Сборка в режиме отладки (с консольным окном)
./build.sh debug

# Сборка в release режиме (оптимизированная)
./build.sh release
```

Скрипт автоматически:
- Определяет доступный компилятор (MSVC или MinGW)
- Создает недостающие заголовочные файлы-заглушки
- Собирает проект с правильными флагами
- Предлагает запустить приложение после сборки

### Ручная сборка

#### С помощью MSVC (Visual Studio)

```cmd
# Debug режим
cl /std:c++17 /EHsc /D_DEBUG /DDEBUG /Zi /Od /MDd main.cpp MainWindow.cpp /Fe:winshluha.exe /link /DEBUG /SUBSYSTEM:CONSOLE user32.lib gdi32.lib d2d1.lib dwrite.lib gdiplus.lib mf.lib mfplat.lib mfreadwrite.lib mfuuid.lib

# Release режим
cl /std:c++17 /EHsc /O2 /MD /DNDEBUG main.cpp MainWindow.cpp /Fe:winshluha.exe /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib d2d1.lib dwrite.lib gdiplus.lib mf.lib mfplat.lib mfreadwrite.lib mfuuid.lib
```

#### С помощью MinGW

```bash
# Debug режим
x86_64-w64-mingw32-g++ -std=c++17 -g -O0 -D_DEBUG -DDEBUG -mconsole main.cpp MainWindow.cpp -o winshluha.exe -mconsole -static-libgcc -static-libstdc++ -luser32 -lgdi32 -ld2d1 -ldwrite -lgdiplus -lmf -lmfplat -lmfreadwrite -lmfuuid

# Release режим
x86_64-w64-mingw32-g++ -std=c++17 -O2 -DNDEBUG -mwindows main.cpp MainWindow.cpp -o winshluha.exe -mwindows -static-libgcc -static-libstdc++ -luser32 -lgdi32 -ld2d1 -ldwrite -lgdiplus -lmf -lmfplat -lmfreadwrite -lmfuuid
```

## Использование

### Управление

- **G** - Открыть/закрыть анимированное меню
- **ESC** - Закрыть меню (если открыто)

### Режимы сборки

- **Debug**: Включает консольное окно для отладки, символы отладки и дополнительную информацию
- **Release**: Оптимизированная сборка без отладочной информации

## Структура проекта

```
winshluha/
├── main.cpp           # Точка входа приложения
├── MainWindow.h       # Заголовочный файл главного окна
├── MainWindow.cpp     # Реализация главного окна
├── build.sh           # Скрипт автоматической сборки
├── README.md          # Этот файл
└── .gitignore         # Исключения для Git
```

## Архитектура

- **main.cpp**: Инициализация COM, Media Foundation, GDI+ и главного окна
- **MainWindow**: Управление окном, рендеринг Direct2D, обработка событий
- **CameraView**: Работа с камерой через Media Foundation (автогенерируется)
- **AnimatedMenu**: Анимированное пользовательское меню (автогенерируется)
- **DebugOverlay**: Отладочная информация (только в debug режиме, автогенерируется)

## Troubleshooting

### Ошибки компиляции

1. **Отсутствуют заголовочные файлы**: Скрипт build.sh автоматически создаст заглушки
2. **Ошибка "cl not found"**: Запустите из Developer Command Prompt для VS
3. **Ошибка "mingw not found"**: Установите MinGW-w64

### Ошибки выполнения

1. **Не инициализируется COM**: Убедитесь, что приложение запущено в правильном контексте
2. **Не работает Media Foundation**: Проверьте поддержку камеры в системе
3. **Проблемы с рендерингом**: Убедитесь в поддержке Direct2D

## Лицензия
MIT
