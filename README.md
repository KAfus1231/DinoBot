# 🦖 Dino Bot — Автомат для Google Chrome Dino на C++ и OpenCV

Этот бот играет в скрытую игру Google Chrome "Dino" полностью автономно, используя **захват экрана через WinAPI**, **обработку изображений с OpenCV** и **эмуляцию клавиш**.

---

## 🚀 Возможности

- 🎮 Реагирует на препятствия (кактусы, птицы)
- 🌙 Поддержка дневного и ночного режима
- ⚙️ Автокалибровка сложности во времени
- 🔍 Распознавание через HSV и контуры
- 🧱 Простая настройка через `Config`-структуру
- 🖥 Захват экрана через WinAPI (`hwnd2mat`)

---

## 📦 Зависимости

- [OpenCV](https://opencv.org/) (рекомендуется OpenCV 4.x)
- Windows (WinAPI)

---

## 🔧 Сборка

### Через CMake (предпочтительно):

```bash
git clone https://github.com/KAfus1231/DinoBot.git
cd dino-bot-opencv
mkdir build && cd build
cmake ..
cmake --build .
